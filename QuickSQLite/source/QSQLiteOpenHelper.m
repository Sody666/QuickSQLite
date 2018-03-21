//
//  SQLiteOpenHelper.m
//  SQLite
//
//  Created by Sou Dai on 13-5-29.
//

#import "QSQLiteOpenHelper.h"
#import "QDBValue.h"
#import "QDBException.h"
#import "QSQLite.h"

#define kQDBPath ([NSSearchPathForDirectoriesInDomains(NSLibraryDirectory, NSUserDomainMask, YES) objectAtIndex:0])
#define kQDBDirectory @"QDatabases"


@interface QDBValue(helper)
/**
 Prepare an array of QDBValues with pairs of key and value.
 for update or insert use.
 
 @param keyValues key and value for column name and value
 @return prepared values
 */
+(NSArray*)valuesWithDictionary:(const NSDictionary*)keyValues;


/**
 Prepare an array of QDBValues with columns.
 For query use.
 
 @param columns target columns
 @return prepared values
 */
+(NSArray*)valuesWithColumns:(const NSArray*)columns;

/**
 Mapping the content value for query string.
 Note: if you don't need some kinds of format, just provide nil.
 for example, set updateOutput and insertOutput to nil if you want
 query format only.
 
 @param contentValues values
 @param queryOutput as name hints
 @param updateOutput as name hints
 @param insertOutput as name hints
 */
+(void)generateSQLWithValues:(const NSArray<QDBValue*>*)contentValues
                       query:(NSString**)queryOutput
                      update:(NSString**)updateOutput
                      insert:(NSString**)insertOutput;



/**
 *  unbind values from statement.
 *  For inner use.
 *
 *  @param values value to bind to
 *  @param stmt   value to bind from
 *  @return whether row of data. nil if failed.
 */
+(NSDictionary*)unbindRowIntoDictionaryWithValues:(const NSArray<QDBValue*> *)values
                                    fromStatement:(sqlite3_stmt *)stmt;


/**
 *  Bind values to statement.
 *
 *  @param values value to bind to
 *  @param stmt   value to bind from
 */
+(void)bindRowWithValues:(const NSArray *)values intoStatement:(sqlite3_stmt *)stmt;
@end

@interface QSQLiteOpenHelper ()
@property (nonatomic, readonly) sqlite3* currentDatabase;
@property (nonatomic, strong) NSString* databaseName;
@property (nonatomic, assign) int databaseVersion;
@property (weak) id<QSQLiteOpenHelperDelegate>openDelegate;
@property (nonatomic, assign) QDBPageSize pageSize;
@end

#define CLOSE_DB(db) do{if((db) != NULL){sqlite3_close((db)); (db)=NULL;} }while(0)

@implementation QSQLiteOpenHelper
@synthesize currentDatabase = _currentDatabase;

- (id)initWithName:(NSString *)name
      openDelegate:(id)delegate{
    return [self initWithName:name version:0 openDelegate:delegate];
}

- (id)initWithName:(const NSString *)name
           version:(const int)version
      openDelegate:(id)delegate
{
    return [self initWithName:name
                          key:nil
                      version:version
                     pageSize:QDBPageSizeDefault
                 openDelegate:delegate];
}

- (id)initWithName:(const NSString *)name
               key:(const NSString*)key
      openDelegate:(id)delegate{
    return [self initWithName:name
                          key:key
                      version:0
                     pageSize:QDBPageSizeDefault
                 openDelegate:delegate];
}

- (id)initWithName:(const NSString *)name
               key:(const NSString*)key
           version:(int)version
          pageSize:(QDBPageSize)pageSize
      openDelegate:(id)delegate{
    self = [super init];
    if (self) {
        _databaseName = name;
        _currentDatabase = NULL;
        _databaseVersion = version;
        _openDelegate = delegate;
        _pageSize = pageSize;
        [self _validDatabaseWithKey:key];
        [self _openCurrentDatabaseWithKey:key];
    }
    
    return self;
}

#pragma mark -- Valid current database

-(BOOL)_isValidDB:(sqlite3*)db{
    if(db == NULL){
        return NO;
    }
    
    sqlite3_stmt* stmt = NULL;
    BOOL isValid = NO;
    if(sqlite3_prepare_v2(db, "PRAGMA user_version;", -1, &stmt, NULL) == SQLITE_OK) {
        if(sqlite3_step(stmt)== SQLITE_ROW) {
            const unsigned char *ver = sqlite3_column_text(stmt, 0);
            if(ver != NULL) {
                isValid = YES;
            }
        }
        sqlite3_finalize(stmt);
    }
    
    return isValid;
}

-(sqlite3*)_openDatabaseInPath:(NSString*)path withKey:(NSString*)key{
    sqlite3* result = NULL;
    if(path.length == 0){
        return result;
    }
    
    BOOL existed = [[NSFileManager defaultManager] fileExistsAtPath:path];
    if(sqlite3_open([path UTF8String], &result) != SQLITE_OK){
        return NULL;
    }
    
    if(key.length > 0){
        const char* utf8Key = [key UTF8String];
        unsigned long keyLength = strlen(utf8Key);
        if(existed){
            sqlite3_key(result, utf8Key, (int)keyLength);
            [self runPragma:[NSString stringWithFormat:@"cipher_page_size = %lu", (unsigned long)self.pageSize] forDB:result];
        }
    }
    
    if(![self _isValidDB:result]){
        CLOSE_DB(result);
    }
    
    return result;
}

-(NSString*)_databaseDiretory{
    NSString* folder = [NSString stringWithFormat:@"%@/%@/", kQDBPath, kQDBDirectory];
    
    NSFileManager* fileManager = [NSFileManager defaultManager];
    
    NSError* error;
    if (![fileManager fileExistsAtPath:folder]) {
        [fileManager createDirectoryAtPath:folder withIntermediateDirectories:YES attributes:nil error:&error];
    }
    
    if(error){
        @throw [QDBException exceptionForReason:error.localizedFailureReason userInfo:nil];
    }
    
    return folder;
}


#define THROW_EXCEPTION(reason,key) do{\
    NSString* fullReason = nil;\
    if(key.length > 0){\
        fullReason = QFormatString(@"%@ claimed to be encrypted", reason);\
    }else{\
        fullReason = QFormatString(@"%@ claimed to be clear", reason);\
    }\
\
    @throw [QDBException exceptionForReason:fullReason userInfo:nil];\
}while(0)
- (void)_validDatabaseWithKey:(NSString*)key
{
    NSString	*dbPath = [self _databaseDiretory];
    sqlite3		*sandboxDB = NULL;
    sqlite3     *bundleDB = NULL;
    NSString    *fullPath = [NSString stringWithFormat:@"%@/%@", dbPath, self.databaseName];
    NSString    * tempFullPath = [NSString stringWithFormat:@"%@tmp", fullPath];
    NSFileManager* fileManager = [NSFileManager defaultManager];
    
    
    // try to open sandbox database
    if ([fileManager fileExistsAtPath:fullPath]) {
        sandboxDB = [self _openDatabaseInPath:fullPath withKey:key];
        if (sandboxDB == NULL) {
            THROW_EXCEPTION(@"Failed to open sandbox database file", key);
        }
        
        if(self.databaseVersion <= [self versionForDatabase:sandboxDB]){
            // everything is OK.
            // No need to do any creating or upgrading job
            CLOSE_DB(sandboxDB);
            return;
        }
    }
    
    // try to open bundle database
    if([self.openDelegate respondsToSelector:@selector(pathToCopyBundleDBFileForSQLiteOpenHelper:withName:)]){
        NSString* bundleDBPath = [self.openDelegate pathToCopyBundleDBFileForSQLiteOpenHelper:self
                                                                                     withName:self.databaseName];
        
        if(bundleDBPath.length > 0){
            if(![fileManager fileExistsAtPath:bundleDBPath]){
                @throw [QDBException exceptionForReason:@"Database file is invalid." userInfo:@{@"path":bundleDBPath}];
            }
            
            [fileManager copyItemAtPath:bundleDBPath toPath:tempFullPath error:nil];
            
            bundleDB = [self _openDatabaseInPath:tempFullPath withKey:key];
            if (bundleDB == NULL) {
                THROW_EXCEPTION(@"Failed to open bundle database file", key);
            }
        }
    }
    
    // validating database
    if(sandboxDB != NULL && bundleDB != NULL) {
        // comparing version
        int bundleVersion = [self versionForDatabase:bundleDB];
        int sandboxVersion = [self versionForDatabase:sandboxDB];
        
        if(bundleVersion > sandboxVersion){
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
                    @throw [QDBException exceptionForReason:@"Migration failed" userInfo:nil];
                }
            }else{
                if(bundleVersion > sandboxVersion) {
                    CLOSE_DB(sandboxDB);
                    CLOSE_DB(bundleDB);
                    [fileManager moveItemAtPath:tempFullPath toPath:fullPath error:nil];
                }
            }
        }
    } else if(sandboxDB == NULL && bundleDB != NULL) {
        CLOSE_DB(sandboxDB);
        CLOSE_DB(bundleDB);
        [fileManager moveItemAtPath:tempFullPath toPath:fullPath error:nil];
    } else if(sandboxDB == NULL && bundleDB == NULL) {
        sandboxDB = [self _openDatabaseInPath:fullPath withKey:key];
        if (sandboxDB == NULL) {
            THROW_EXCEPTION(@"Failed to create database file", key);
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
    
    
    // upgrading...
    // 心好累啊，jb requirements
    sandboxDB = [self _openDatabaseInPath:fullPath withKey:key];
    if (sandboxDB == NULL) {
        THROW_EXCEPTION(@"Failed to open database file", key);
    }
    
    int oldVersion = [self versionForDatabase:sandboxDB];
    if (oldVersion < self.databaseVersion) {
        if([self.openDelegate respondsToSelector:@selector(SQLiteOpenHelper:upgradingDB:fromVersion:toVersion:)]) {
            [self.openDelegate SQLiteOpenHelper:self upgradingDB:sandboxDB fromVersion:oldVersion toVersion:self.databaseVersion];
        }
        
        [self updateDatabase:sandboxDB toVersion:self.databaseVersion];
    }
    
    CLOSE_DB(sandboxDB);
}


-(void)_openCurrentDatabaseWithKey:(NSString*)key{
    if(_currentDatabase != NULL){
        return;
    }
    
    NSString* currentDatabasePath = [NSString stringWithFormat:@"%@/%@",[self _databaseDiretory], self.databaseName];
    _currentDatabase = [self _openDatabaseInPath:currentDatabasePath withKey:key];
    if (_currentDatabase == NULL) {
        @throw [QDBException exceptionForReason:@"Failed to open database fiel" userInfo:@{@"path":currentDatabasePath}];
    }
}

-(void)runPragma:(NSString*)pragma forDB:(sqlite3*)db{
    if(pragma.length == 0){
        return;
    }
    
    NSString *query = [NSString stringWithFormat:@"PRAGMA %@", pragma];
    
    sqlite3_exec(db, [query UTF8String], NULL, NULL, NULL);
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
    [self runPragma:[NSString stringWithFormat:@"user_version = %d", version] forDB:db];
}





#pragma  mark - traditional SQL toolkit
- (NSArray*)query:(const NSString *)tableName
      columns:(const NSArray *)columns
        where:(const NSString *)where
    statement:(sqlite3_stmt **)statement{
    return [self query:tableName columns:columns where:where orderBy:nil limit:nil groupBy:nil statement:statement];
}

- (NSArray*)query:(const NSString *)tableName
     columns:(const NSArray *)columns
       where:(const NSString *)where
     orderBy:(const NSString *)orderBy
       limit:(const NSString *)limit
     groupBy:(const NSString *)groupBy
   statement:(sqlite3_stmt **)statement
{
    NSMutableDictionary* values = [[NSMutableDictionary alloc] initWithCapacity:columns.count];
    
    for (NSString* key in columns) {
        [values setObject:@"" forKey:key];
    }
    
    NSArray* contentValues = [QDBValue valuesWithColumns:columns];
    
	NSMutableString *sqlQuery	= [NSMutableString stringWithString:@"SELECT "];

    NSString* query = nil;
    [QDBValue generateSQLWithValues:contentValues query:&query update:nil insert:nil];
    
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

    if(sqlite3_prepare_v2(self.currentDatabase, [sqlQuery UTF8String], -1, statement, NULL) == SQLITE_OK){
        return contentValues;
    }else{
        return nil;
    }
}

- (long)update:(const NSString *)tableName values:(const NSDictionary*)values where:(const NSString *)where
{
    if(values.count < 1){
        return -1;
    }
    
	NSMutableString *sql	= [NSMutableString stringWithFormat:@"UPDATE %@ SET ", tableName];
	sqlite3_stmt	*stmt;
	int				result;

    NSString* update = nil;
    NSArray* contentValues = [QDBValue valuesWithDictionary:values];
    [QDBValue generateSQLWithValues:contentValues query:nil update:&update insert:nil];
    
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

- (long long)insert:(const NSString *)tableName values:(const NSDictionary*)values
{
    if(values.count < 1){
        return -1;
    }
    
	NSMutableString *sql		= [NSMutableString stringWithFormat:@"INSERT INTO %@( ", tableName];
	NSMutableString *valueSql	= [NSMutableString stringWithFormat:@" VALUES( "];
	long long			result;
	sqlite3_stmt	*stmt;

    NSString* query;
    NSString* insert;
    NSArray* contentValues = [QDBValue valuesWithDictionary:values];
    [QDBValue generateSQLWithValues:contentValues query:&query update:nil insert:&insert];

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
    
    NSArray* contentValues = [self query:tableName
                                 columns:columns
                                   where:where
                                 orderBy:orderBy
                                   limit:limit
                                 groupBy:groupBy
                               statement:&statement];
    
    if(contentValues != nil){
        while ((row=[QDBValue unbindRowIntoDictionaryWithValues:contentValues fromStatement:statement]) != nil) {
            [result addObject:row];
        }
    }
    
    return result;
}
#pragma mark - transaction
-(BOOL)beginTransactionWithError:(NSError**)errorOutput{
    char* error = NULL;
    
    sqlite3_exec(self.currentDatabase, "BEGIN TRANSACTION", NULL, NULL, &error);
    
    if(errorOutput != nil && error != NULL){
        *errorOutput = [NSError errorWithDomain:@"DBOperation" code:0 userInfo:@{@"reason":[NSString stringWithUTF8String:error]}];
    }

    if(error != NULL){
        sqlite3_free(error);
        return NO;
    }else{
        return YES;
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
    CLOSE_DB(_currentDatabase);
}
@end
