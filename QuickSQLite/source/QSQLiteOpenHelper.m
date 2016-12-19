//
//  SQLiteOpenHelper.m
//  SQLite
//
//  Created by Sou Dai on 13-5-29.
//

#import "QSQLiteOpenHelper.h"
#import "QDBValue.h"

#define kQDBPath ([NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES) objectAtIndex:0])
#define kQDBDirectory @"QDatabases"


@interface QSQLiteOpenHelper ()
@property (nonatomic, readonly) sqlite3* currentDatabase;
@property (nonatomic, strong) NSString* databaseName;
@property (nonatomic, assign) int databaseVersion;
@property (weak) id<QSQLiteOpenHelperDelegate>openDelegate;

@end

@implementation QSQLiteOpenHelper
@synthesize currentDatabase = _currentDatabase;

- (id)initWithName:(NSString *)name version:(int)version openDelegate:(id)delegate
{
    self = [super init];
    if (self) {
        _databaseName = name;
        _currentDatabase = NULL;
        _databaseVersion = version;
        _openDelegate = delegate;
        [self validDatabase];
    }

	return self;
}

#pragma mark -- Valid current database

-(NSString*)_databaseDiretory{
    NSString* folder = [NSString stringWithFormat:@"%@/%@/", kQDBPath, kQDBDirectory];
    
    NSFileManager* fileManager = [NSFileManager defaultManager];
    
    NSError* error;
    if (![fileManager fileExistsAtPath:folder]) {
        [fileManager createDirectoryAtPath:folder withIntermediateDirectories:YES attributes:nil error:&error];
    }
    
    if(error){
        @throw [NSException exceptionWithName:@"Quick SQLite openning database" reason:error.localizedFailureReason userInfo:nil];
    }
    
    return folder;
}

#define CLOSE_DB(db) do{if((db) != NULL){sqlite3_close((db)); (db)=NULL;} }while(0)
- (void)validDatabase
{
    NSString	*dbPath = [self _databaseDiretory];
    sqlite3		*sandboxDB = NULL;
    sqlite3     *bundleDB = NULL;
    NSString    *fullPath = [NSString stringWithFormat:@"%@/%@", dbPath, self.databaseName];
    NSString    * tempFullPath = [NSString stringWithFormat:@"%@tmp", fullPath];
    NSFileManager* fileManager = [NSFileManager defaultManager];
    
    // try to open bundle database
    if([self.openDelegate respondsToSelector:@selector(pathToCopyBundleDBFileForSQLiteOpenHelper:withName:)]){
        NSString* bundleDBPath = [self.openDelegate pathToCopyBundleDBFileForSQLiteOpenHelper:self
                                                                           withName:self.databaseName];
        
        if(bundleDBPath.length > 0){
            if(![fileManager fileExistsAtPath:bundleDBPath]){
                @throw [NSException exceptionWithName:@"Quick SQLite validating database" reason:@"Path of bundle database file is invalid" userInfo:nil];
            }
            
            [fileManager copyItemAtPath:bundleDBPath toPath:tempFullPath error:nil];
            
            if (sqlite3_open([tempFullPath UTF8String], &bundleDB) != SQLITE_OK) {
                @throw [NSException exceptionWithName:@"Quick SQLite validating database" reason:@"Failed to open bundle database file" userInfo:nil];
            }
        }
    }

    // try to open sandbox database
    if ([fileManager fileExistsAtPath:fullPath]) {
        if (sqlite3_open([fullPath UTF8String], &sandboxDB) != SQLITE_OK) {
            @throw [NSException exceptionWithName:@"Quick SQLite validating database" reason:@"Failed to open sandbox database file" userInfo:nil];
        }
    }
    
    // validating database
    if(sandboxDB != NULL && bundleDB != NULL) {
        // comparing version
        int bundleVersion = [self versionForDatabase:bundleDB];
        int sandboxVersion = [self versionForDatabase:sandboxDB];
        
        if([self.openDelegate respondsToSelector:@selector(SQLiteOpenHelper:copyingBundleDB:withVersion:fromOldDB:withVersion:)]){
            if([self.openDelegate SQLiteOpenHelper:self copyingBundleDB:bundleDB withVersion:bundleVersion fromOldDB:sandboxDB withVersion:sandboxVersion]){
                CLOSE_DB(sandboxDB);
                CLOSE_DB(bundleDB);
                [fileManager removeItemAtPath:fullPath error:nil];
                [fileManager moveItemAtPath:tempFullPath toPath:fullPath error:nil];
            }else{
                CLOSE_DB(sandboxDB);
                CLOSE_DB(bundleDB);
                [fileManager removeItemAtPath:tempFullPath error:nil];
                @throw [NSException exceptionWithName:@"Quick SQLite validating " reason:@"Migration failed" userInfo:nil];
            }
        }else{
            if(bundleVersion > sandboxVersion) {
                CLOSE_DB(sandboxDB);
                CLOSE_DB(bundleDB);
                [fileManager moveItemAtPath:tempFullPath toPath:fullPath error:nil];
            }
        }
    } else if(sandboxDB == NULL && bundleDB != NULL) {
        CLOSE_DB(sandboxDB);
        CLOSE_DB(bundleDB);
        [fileManager moveItemAtPath:tempFullPath toPath:fullPath error:nil];
    } else if(sandboxDB == NULL && bundleDB == NULL) {
        if (sqlite3_open([fullPath UTF8String], &sandboxDB) != SQLITE_OK) {
            @throw [NSException exceptionWithName:@"Quick SQLite validating database" reason:@"Creating database failed" userInfo:nil];
        }
        
        BOOL shouldSave = NO;
        if([self.openDelegate respondsToSelector:@selector(SQLiteOpenHelper:creatingDB:)]){
            shouldSave = [self.openDelegate SQLiteOpenHelper:self creatingDB:sandboxDB];
        }
        
        CLOSE_DB(sandboxDB);
        
        if(!shouldSave){
            [fileManager removeItemAtPath:fullPath error:nil];
        }
    }/*else if(db != NULL && bundleDB == NULL)*/
    
    CLOSE_DB(sandboxDB);
    CLOSE_DB(bundleDB);
}

/**
 *   Check the database file. If it doesn't exist, create one brand new.
 *   Please Note that the new database don't have any information.
 *
 *   Will throw a exception if creating new database fails
 */
- (sqlite3 *)currentDatabase
{
    if (_currentDatabase == NULL) {
        int oldVersion;

        if (sqlite3_open([[NSString stringWithFormat:@"%@/%@",[self _databaseDiretory], self.databaseName ] UTF8String], &_currentDatabase) != SQLITE_OK) {
            @throw [NSException exceptionWithName:@"Quick SQLite openning database" reason:@"Openning sqlite3 database failed" userInfo:nil];
            return NULL;
        }

        oldVersion = [self versionForDatabase:_currentDatabase];


        if (oldVersion != self.databaseVersion) {
            if(oldVersion != 0 && [self.openDelegate respondsToSelector:@selector(SQLiteOpenHelper:upgradingDB:fromVersion:toVersion:)]) {
                [self.openDelegate SQLiteOpenHelper:self upgradingDB:_currentDatabase fromVersion:oldVersion toVersion:self.databaseVersion];
            }
            
            [self updateDatabase:_currentDatabase toVersion:self.databaseVersion];
        }
    }


	return _currentDatabase;
}

#pragma mark -- database version getter and setter
- (int)versionForDatabase:(sqlite3 *)db
{
	sqlite3_stmt *statement;

	sqlite3_prepare_v2(db, "PRAGMA user_version", -1, &statement, nil);
	sqlite3_step(statement);
	int version = sqlite3_column_int(statement, 0);
	sqlite3_finalize(statement);
	return version;
}

-(void)updateDatabase:(sqlite3*)db toVersion:(int)version
{
	NSString *query = [NSString stringWithFormat:@"PRAGMA user_version = %d", version];

	sqlite3_exec(db, [query UTF8String], NULL, NULL, NULL);
}





#pragma  mark - traditional SQL toolkit
- (BOOL)query:(const NSString *)tableName
      columns:(const NSArray *)columns
        where:(const NSString *)where
    statement:(sqlite3_stmt **)statement{
    return [self query:tableName columns:columns where:where orderBy:nil limit:nil groupBy:nil statement:statement];
}

- (BOOL)query:(const NSString *)tableName
     columns:(const NSArray *)columns
       where:(const NSString *)where
     orderBy:(const NSString *)orderBy
       limit:(const NSString *)limit
     groupBy:(const NSString *)groupBy
   statement:(sqlite3_stmt **)statement
{
	NSMutableString *sqlQuery	= [NSMutableString stringWithString:@"SELECT "];

    NSString* query = nil;
    [QDBValue mappingContentValues:columns query:&query update:nil insert:nil];
    
    [sqlQuery appendString:query];

	[sqlQuery appendFormat:@" FROM %@", tableName];

	if (where.length > 0) {
		[sqlQuery appendFormat:@" WHERE %@", where];
	}

    if (groupBy.length > 0) {
        [sqlQuery appendFormat:@" GROUP BY %@", groupBy];
    }

	if (orderBy.length > 0) {
		[sqlQuery appendFormat:@" ORDER BY %@", orderBy];
	}

	if (limit.length > 0) {
		[sqlQuery appendFormat:@" LIMIT %@", limit];
	}

	[sqlQuery appendString:@";"];

	return sqlite3_prepare_v2(self.currentDatabase, [sqlQuery UTF8String], -1, statement, NULL) == SQLITE_OK;
}

- (long)update:(const NSString *)tableName contentValues:(const NSArray *)contentValues where:(const NSString *)where
{
	NSMutableString *sql	= [NSMutableString stringWithFormat:@"UPDATE %@ SET ", tableName];
	sqlite3_stmt	*stmt;
	int				result;

    NSString* update = nil;
    [QDBValue mappingContentValues:contentValues query:nil update:&update insert:nil];
    
    [sql appendString:update];

    if (where) {
        [sql appendFormat:@" WHERE %@", where ];
    }

	[sql appendString:@";"];

	sqlite3_prepare_v2(self.currentDatabase, [sql UTF8String], -1, &stmt, NULL);

    [QDBValue bindRowWithValues:contentValues intoStatement:stmt];

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        result = sqlite3_changes(self.currentDatabase);
    }else{
        result = 0;
    }
	sqlite3_finalize(stmt);

	return result;
}

- (long long)insert:(const NSString *)tableName contentValues:(const NSArray *)contentValues
{
	NSMutableString *sql		= [NSMutableString stringWithFormat:@"INSERT INTO %@( ", tableName];
	NSMutableString *valueSql	= [NSMutableString stringWithFormat:@" VALUES( "];
	long long			result;
	sqlite3_stmt	*stmt;

    NSString* query;
    NSString* insert;
    [QDBValue mappingContentValues:contentValues query:&query update:nil insert:&insert];

    [sql appendString:query];
    [valueSql appendString:insert];

	[valueSql appendString:@");"];
	[sql appendFormat:@") %@", valueSql];

	sqlite3_prepare_v2(self.currentDatabase, [sql UTF8String], -1, &stmt, NULL);

    [QDBValue bindRowWithValues:contentValues intoStatement:stmt];

	if (sqlite3_step(stmt) == SQLITE_DONE) {
		result = sqlite3_last_insert_rowid(self.currentDatabase);
	} else {
		result = -1;
	}

	sqlite3_finalize(stmt);

	return result;
}

- (long)remove:(const NSString *)tableName where:(const NSString *)where
{
	NSString *sql;

	if (where != nil) {
		sql = [NSString stringWithFormat:@"DELETE FROM %@ WHERE %@;", tableName, where];
	} else {
		sql = [NSString stringWithFormat:@"DELETE FROM %@;", tableName];
	}
    
    long result;
    sqlite3_stmt	*stmt;
    sqlite3_prepare_v2(self.currentDatabase, [sql UTF8String], -1, &stmt, NULL);
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        result = sqlite3_changes(self.currentDatabase);
    }else{
        result = 0;
    }
    sqlite3_finalize(stmt);

	return result;
}
#pragma mark - enhanced SQL
-(BOOL)isRecordAvailableInTable:(const NSString*)tableName
                     primaryKey:(const NSString*)primaryKey
                      condition:(const NSString*)where{
    return [self recordCountInTable:tableName primaryKey:primaryKey condition:where] > 0;
}

-(long)recordCountInTable:(const NSString*)tableName
               primaryKey:(const NSString*)primaryKey
                condition:(const NSString*)where{
    NSString* column = [NSString stringWithFormat:@"count(%@)", primaryKey];
    
    NSArray* result = [self query:tableName columns:@[column] where:where orderBy:nil limit:nil groupBy:nil];
    if(result.count == 0){
        return 0;
    }else{
        NSDictionary* row = result.firstObject;
        NSNumber* count = [row objectForKey:column];
        if(count == nil){
            return 0;
        }else{
            return count.longValue;
        }
    }
}

-(NSArray*)query:(const NSString *)tableName
         columns:(const NSArray *)columns
           where:(const NSString *)where{
    return [self query:tableName columns:columns where:where orderBy:nil limit:nil groupBy:nil];
}

-(NSArray*)query:(const NSString *)tableName
         columns:(const NSArray *)columns
           where:(const NSString *)where
         orderBy:(const NSString *)orderBy
           limit:(const NSString *)limit
         groupBy:(const NSString *)groupBy{
    sqlite3_stmt* statement = NULL;
    NSMutableArray* result = [[NSMutableArray alloc] init];
    NSDictionary* row;
    
    NSMutableArray* contentValues = [[NSMutableArray alloc] init];
    for (NSString* key in columns) {
        [contentValues addObject:[QDBValue instanceWithKey:key]];
    }
    
    
    if([self query:tableName
           columns:contentValues
             where:where
           orderBy:orderBy
             limit:limit
           groupBy:groupBy
         statement:&statement]){
        
        
        while ((row=[QDBValue unbindRowIntoDictionaryWithValues:contentValues fromStatement:statement]) != nil) {
            [result addObject:row];
        }
    }
    
    return result;
}
#pragma mark - transaction
-(void)beginTransactionWithError:(NSError**)errorOutput{
    char* error = NULL;
    
    if(errorOutput == nil){
        sqlite3_exec(self.currentDatabase, "BEGIN TRANSACTION", NULL, NULL, NULL);
    }else{
        sqlite3_exec(self.currentDatabase, "BEGIN TRANSACTION", NULL, NULL, &error);
    }
    
    if(error != NULL){
        *errorOutput = [NSError errorWithDomain:@"DBOperation" code:0 userInfo:@{@"reason":[NSString stringWithUTF8String:error]}];
        
        sqlite3_free(error);
    }
}

-(void)endTransaction{
    sqlite3_exec(self.currentDatabase, "END TRANSACTION", NULL, NULL, NULL);
}
-(void)rollbackTransaction{
    sqlite3_exec(self.currentDatabase, "ROLLBACK", NULL, NULL, NULL);
}
#pragma mark - other functions
- (void)dealloc
{
    [self close];
}

-(void)close{
    sqlite3_close(_currentDatabase);
    _currentDatabase = NULL;
}
@end