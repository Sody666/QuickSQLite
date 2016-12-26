//
//  SQLiteOpenHelper.h
//  SQLite
//
//  Created by Sou Dai on 13-5-29.
//

#import <Foundation/Foundation.h>
#import <sqlite3.h>

@class QSQLiteOpenHelper;
@class QDBValue;
@protocol QSQLiteOpenHelperDelegate <NSObject>
@optional
/**
 *  Delegate for upgrading database.
 *
 *  @param openHelper event sender
 *  @param database   target database
 *  @param oVersion   old version
 *  @param nVersion   new version
 */
-(void) SQLiteOpenHelper:(QSQLiteOpenHelper*)openHelper
             upgradingDB:(sqlite3 *)database
             fromVersion:(int)oVersion
               toVersion:(int)nVersion;

/**
 *  Delegate for creating tables for brand new database.
 *  This will be tried to be called if no database is found
 *  before copying.
 *
 *  @param openHelper event sender
 *  @param database   target database created moments ago
 *
 *  @return whether the database should be saved
 */
-(BOOL) SQLiteOpenHelper:(QSQLiteOpenHelper*)openHelper
              creatingDB:(sqlite3 *)database;



/**
 Migrate old data to new database.
 

 @param openHelper event sender
 @param bundleDatabase database which will take place of old db.
 @param bversion bundler db version
 @param oldDatabase old database
 @param oVersion old database version
 @return Whether should save bundle file. If YES, Old db will be delete and new one will be saved.
                                          If NO, an exception will be raised.
 */
-(BOOL) SQLiteOpenHelper:(QSQLiteOpenHelper *)openHelper
         copyingBundleDB:(sqlite3 *)bundleDatabase
             withVersion:(int)bversion
               fromOldDB:(sqlite3 *)oldDatabase
             withVersion:(int)oVersion;

/**
 Request the path to copy bundle dabase file.

 @param openHelper event sender
 @param name    name of the database manipulating
 @return the path of the bundle file
 */
-(NSString*) pathToCopyBundleDBFileForSQLiteOpenHelper:(QSQLiteOpenHelper *)openHelper
                                              withName:(NSString*)name;
@end

@interface QSQLiteOpenHelper : NSObject

/**
 *  Initialize the database with name and version.
 *
 *  @param name    database name
 *  @param version database version
 *
 *  @return helper initialized
 */
- (id)initWithName:(NSString *)name version:(int)version openDelegate:(id)delegate;
#pragma mark - traditional sql interface
/**
 Do query on the table. This api should be use if 
 the result is quite large, because you can 'draw' 
 the data little by little, for your need.
 
 Else you can use the one without statement.

 @param tableName target table
 @param columns columns you want to select
 @param where where condition
 @param orderBy order by condition
 @param limit limit condition
 @param groupBy group by condition
 @param statement statement to unbind data
 @return array the QDBValue where you can unbind data. nil if failed.
 */
-(NSArray<QDBValue*>*)query:(const NSString *)tableName
                    columns:(const NSArray<NSString*> *)columns
                      where:(const NSString *)where
                    orderBy:(const NSString *)orderBy
                      limit:(const NSString *)limit
                    groupBy:(const NSString *)groupBy
                  statement:(sqlite3_stmt **)statement;

/**
 Do query on the table. This api should be use if
 the result is quite large, because you can 'draw'
 the data little by little, for your need.
 
 Else you can use the one without statement.
 
 @param tableName target table
 @param columns columns you want to select
 @param where where condition
 @param statement statement to unbind data
 @return array the QDBValue where you can unbind data. nil if failed.
 */
-(NSArray<QDBValue*>*)query:(const NSString *)tableName
                    columns:(const NSArray<NSString*> *)columns
                      where:(const NSString *)where
                  statement:(sqlite3_stmt **)statement;

/**
 *    Update records with condition.
 *    @param tableName table where the query happens
 *    @param values updating values
 *    @param where where condition
 *    @return count of record affected
 */
-(long)update:(const NSString *)tableName
       values:(const NSDictionary*)values
        where:(const NSString *)where;

/**
 *    Insert a value.
 *    @param tableName table where the query happens
 *    @param values inserting value
 *    @return primary id of record saved
 */
-(long long)insert:(const NSString *)tableName
            values:(const NSDictionary*)values;

/**
 *    remove values
 *    @param tableName where the query happens
 *    @param where where condition
 *    @return count of record affacted
 */
-(long)remove:(const NSString *)tableName
        where:(const NSString *)where;

#pragma mark - enhanced SQL interface

/**
 Detect whether the target record is in the table

 @param tableName target table
 @param primaryKey primary key for the table
 @param where where condition
 @return whether any record for the condition exists
 */
-(BOOL)isRecordAvailableInTable:(const NSString*)tableName
                     primaryKey:(const NSString*)primaryKey
                      condition:(const NSString*)where;

/**
 Detect count of the target record in the table
 
 @param tableName target table
 @param primaryKey primary key for the table
 @param where where condition
 @return record count for the condition
 */
-(long)recordCountInTable:(const NSString*)tableName
               primaryKey:(const NSString*)primaryKey
                condition:(const NSString*)where;

/**
 *    Do a query on the database.
 *    @param tableName table where the query happens
 *    @param columns columns for query
 *    @param where where condition
 *    @param orderBy orderBy condition
 *    @param limit limit condition
 *    @return rows wrapped by dict, keyed by column name.
 */
-(NSArray<NSDictionary*>*)query:(const NSString *)tableName
                        columns:(const NSArray<NSString*> *)columns
                          where:(const NSString *)where
                        orderBy:(const NSString *)orderBy
                          limit:(const NSString *)limit
                        groupBy:(const NSString *)groupBy;

/**
 *    Do a query on the database.
 *    @param tableName table where the query happens
 *    @param columns columns for query
 *    @param where where condition
 *    @return rows wrapped by dict, keyed by column name.
 */
-(NSArray<NSDictionary*>*)query:(const NSString *)tableName
                        columns:(const NSArray<NSString*> *)columns
                          where:(const NSString *)where;
#pragma mark - other tools
/**
 Force database to be closed.
 */
-(void)close;

#pragma mark - transation series

/**
 Start the transaction.
 After transaction, do what ever change action to database as usual.
 Example:
 [help beginTransactionWithError:nil];
 [help insert...];
 [help update...];
 [help remove...];
 [help endTransaction];
 
 
 Transactional control commands are only used with the DML 
 commands INSERT, UPDATE, and DELETE. They can not be used 
 while creating tables or dropping them because these 
 operations are automatically committed in the database.
 Transactions usually persist until the next COMMIT or ROLLBACK 
 command encountered. But a transaction will also ROLLBACK 
 if the database is closed or if an error occurs.

 @param errorOutput error if failed. Provide nil if you don't care
 
 @return whether transaction begin successfully
 */
-(BOOL)beginTransactionWithError:(NSError**)errorOutput;


/**
 End transaction.
 
 Saves all transactions to the database since the last COMMIT or ROLLBACK command.
 */
-(void)endTransaction;


/**
 Undo transactions that have not already been saved to the database.
 */
-(void)rollbackTransaction;
@end
