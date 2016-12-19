//
//  DataCenterTests.m
//  QuickSQLite
//
//  Created by sudi on 2016/12/16.
//
//

#import "RootTestCase.h"
#import "QSQLiteOpenHelper.h"
#import "QDBValue.h"

#define kTableName @"person"

#define kColumnId   @"_id"
#define kColumnAge  @"age"
#define kColumnName @"name"
#define kColumnHeight @"height"
#define kColumnAvatar @"avatar"


@interface QSQLiteOpenHelper ()
-(NSString*)_databaseDiretory;
@end


@interface DataCenterTests : RootTestCase <QSQLiteOpenHelperDelegate>
@end

@implementation DataCenterTests

- (void)setUp {
    [super setUp];
    NSString* dbName = @"testDB.sql";
    BindUTData(dbName);
    QSQLiteOpenHelper* helper = [[QSQLiteOpenHelper alloc] initWithName:dbName version:1 openDelegate:self];
    
    NSBundle* bundle = [NSBundle bundleForClass:[self class]];
    NSData* imageData = [NSData dataWithContentsOfFile:[bundle pathForResource:@"sampleSmallImage" ofType:@"png"]];
    
    BindUTData(helper, imageData);
}

- (void)tearDown {
    QSQLiteOpenHelper* helper = TData(@"helper");
    NSString* dbFilePath = [NSString stringWithFormat:@"%@/%@", [helper _databaseDiretory], TData(@"dbName")];
    
    [super tearDown];
    [[NSFileManager defaultManager] removeItemAtPath:dbFilePath error:nil];
}

-(void)testTraditionalSQL{
    QSQLiteOpenHelper* helper = TData(@"helper");
    
    // empty database part
    {
        NSArray* columns = @[kColumnName, kColumnAge, kColumnId];
        
        XCTAssertTrue([helper recordCountInTable:kTableName primaryKey:kColumnId condition:nil]==0, @"应该啥数据都没的");
        XCTAssertFalse([helper isRecordAvailableInTable:kTableName primaryKey:kColumnId condition:nil], @"应该是没记录");
        XCTAssertTrue([helper query:kTableName columns:columns where:nil orderBy:nil limit:nil groupBy:nil].count == 0, @"应该啥数据都没的");
    }
    // insert part
    {
        // insert first record
        NSMutableArray* contentValues = [[NSMutableArray alloc] init];
        [contentValues addObject:[QDBValue instanceForObject:@"Alice" withKey:kColumnName]];
        [contentValues addObject:[QDBValue instanceForObject:@(21) withKey:kColumnAge]];
        XCTAssertTrue([helper insert:kTableName contentValues:contentValues] == 1, @"插入的数据的Primary key应该为1");
        XCTAssertTrue([helper recordCountInTable:kTableName primaryKey:kColumnId condition:nil]==1, @"数据库应该有且仅有一条数据");
        
        // insert second record
        [contentValues removeAllObjects];
        [contentValues addObject:[QDBValue instanceForObject:@"Cindy" withKey:kColumnName]];
        [contentValues addObject:[QDBValue instanceForObject:@(22) withKey:kColumnAge]];
        XCTAssertTrue([helper insert:kTableName contentValues:contentValues] == 2, @"插入的数据的Primary key应该为2");
        XCTAssertTrue([helper recordCountInTable:kTableName primaryKey:kColumnId condition:nil]==2, @"数据库应该有且仅有两条数据");
        
        // insert 3rd record with specified id
        [contentValues removeAllObjects];
        [contentValues addObject:[QDBValue instanceForObject:@"Dick" withKey:kColumnName]];
        [contentValues addObject:[QDBValue instanceForObject:@(27) withKey:kColumnAge]];
        [contentValues addObject:[QDBValue instanceForObject:@(10) withKey:kColumnId]];
        XCTAssertTrue([helper insert:kTableName contentValues:contentValues] == 10, @"插入的数据的Primary key应该为10");
        XCTAssertTrue([helper recordCountInTable:kTableName primaryKey:kColumnId condition:nil]==3, @"数据库应该有且仅有两条数据");
        
        // insert 4th record without id
        [contentValues removeAllObjects];
        [contentValues addObject:[QDBValue instanceForObject:@"Emily" withKey:kColumnName]];
        [contentValues addObject:[QDBValue instanceForObject:@(27) withKey:kColumnAge]];
        XCTAssertTrue([helper insert:kTableName contentValues:contentValues] == 11, @"插入的数据的Primary key应该为11");
        XCTAssertTrue([helper recordCountInTable:kTableName primaryKey:kColumnId condition:nil]==4, @"数据库应该有且仅有两条数据");
    }
    
    // query part
    {
        NSString* where = [NSString stringWithFormat:@"%@==1", kColumnId];
        NSMutableArray<QDBValue*>* contentValues = [[NSMutableArray alloc] init];
        [contentValues addObject:[QDBValue instanceForObject:@"" withKey:kColumnName]];
        [contentValues addObject:[QDBValue instanceForObject:@(0) withKey:kColumnAge]];
        [contentValues addObject:[QDBValue instanceForObject:@(NSIntegerMax) withKey:kColumnId]];
        sqlite3_stmt* statement = NULL;
        XCTAssertTrue([helper query:kTableName columns:contentValues where:where orderBy:nil limit:nil groupBy:nil statement:&statement], @"查询应该要成功");
        XCTAssertTrue([QDBValue unbindRowIntoValues:contentValues fromStatement:statement], @"第一次取数据应该要成功");
        XCTAssertTrue([@"Alice" isEqualToString:[contentValues[0] value]], @"取到的名称应该跟保存的一致");
        XCTAssertTrue(((NSNumber*)[contentValues[1] value]).intValue == 21, @"取到的年龄应该要一致");
        XCTAssertTrue(((NSNumber*)[contentValues[2] value]).longValue == 1, @"取到的身份ID应该是1");
        XCTAssertFalse([QDBValue unbindRowIntoValues:contentValues fromStatement:statement], @"第二次取数据不应该成功");
        sqlite3_finalize(statement);
        
        
        NSArray* rows = [helper query:kTableName columns:@[kColumnName, kColumnAge, kColumnId] where:where orderBy:nil limit:nil groupBy:nil];
        XCTAssertNotNil(rows, @"查询结果不应该为nil");
        XCTAssertTrue(rows.count == 1);
        NSDictionary* row = rows.firstObject;
        XCTAssertTrue([row isKindOfClass:[NSDictionary class]], @"row数据应该是一个字典");
        XCTAssertTrue([@"Alice" isEqualToString:[row objectForKey:kColumnName]], @"取到的名称应该跟保存的一致");
        XCTAssertTrue(((NSNumber*)[row objectForKey:kColumnAge]).intValue == 21, @"取到的年龄应该要一致");
        XCTAssertTrue(((NSNumber*)[row objectForKey:kColumnId]).longValue == 1, @"取到的身份ID应该是1");
    }
    
    // update part
    {
        
        NSMutableArray<QDBValue*>* contentValues = [[NSMutableArray alloc] init];
        [contentValues addObject:[QDBValue instanceForObject:@"Bob" withKey:kColumnName]];
        [contentValues addObject:[QDBValue instanceForObject:@(19) withKey:kColumnAge]];
        
        NSString* where = [NSString stringWithFormat:@"%@==1", kColumnId];
        XCTAssertTrue([helper update:kTableName contentValues:contentValues where:where], @"更新应该要成功");
    }
    
    // query confirm part
    {
        // by id
        NSString* where = [NSString stringWithFormat:@"%@==1", kColumnId];
        NSMutableArray<QDBValue*>* contentValues = [[NSMutableArray alloc] init];
        [contentValues addObject:[QDBValue instanceForObject:@"" withKey:kColumnName]];
        [contentValues addObject:[QDBValue instanceForObject:@(0) withKey:kColumnAge]];
        [contentValues addObject:[QDBValue instanceForObject:@(NSIntegerMax) withKey:kColumnId]];
        sqlite3_stmt* statement = NULL;
        XCTAssertTrue([helper query:kTableName columns:contentValues where:where orderBy:nil limit:nil groupBy:nil statement:&statement], @"查询应该要成功");
        XCTAssertTrue([QDBValue unbindRowIntoValues:contentValues fromStatement:statement], @"第一次取数据应该要成功");
        XCTAssertTrue([@"Bob" isEqualToString:[contentValues[0] value]], @"取到的名称应该跟保存的一致");
        XCTAssertTrue(((NSNumber*)[contentValues[1] value]).intValue == 19, @"取到的年龄应该要一致");
        XCTAssertTrue(((NSNumber*)[contentValues[2] value]).longValue == 1, @"取到的身份ID应该是1");
        XCTAssertFalse([QDBValue unbindRowIntoValues:contentValues fromStatement:statement], @"第二次取数据不应该成功");
        sqlite3_finalize(statement);
        
        where = [NSString stringWithFormat:@"%@=='Dick'", kColumnName];
        contentValues = [[NSMutableArray alloc] init];
        [contentValues addObject:[QDBValue instanceForObject:@"" withKey:kColumnName]];
        [contentValues addObject:[QDBValue instanceForObject:@(0) withKey:kColumnAge]];
        [contentValues addObject:[QDBValue instanceForObject:@(NSIntegerMax) withKey:kColumnId]];
        statement = NULL;
        XCTAssertTrue([helper query:kTableName columns:contentValues where:where orderBy:nil limit:nil groupBy:nil statement:&statement], @"查询应该要成功");
        XCTAssertTrue([QDBValue unbindRowIntoValues:contentValues fromStatement:statement], @"第一次取数据应该要成功");
        XCTAssertTrue([@"Dick" isEqualToString:[contentValues[0] value]], @"取到的名称应该跟保存的一致");
        XCTAssertTrue(((NSNumber*)[contentValues[1] value]).intValue == 27, @"取到的年龄应该要一致");
        XCTAssertTrue(((NSNumber*)[contentValues[2] value]).longValue == 10, @"取到的身份ID应该是1");
        XCTAssertFalse([QDBValue unbindRowIntoValues:contentValues fromStatement:statement], @"第二次取数据不应该成功");
        sqlite3_finalize(statement);
    }
    
    // remove part
    {
        XCTAssertTrue([helper recordCountInTable:kTableName primaryKey:kColumnId condition:nil]==4, @"数据库应该有4条数据");
        NSString* where = [NSString stringWithFormat:@"%@==1", kColumnId];
        XCTAssertTrue([helper remove:kTableName where:where] == 1, @"发生变化的记录条数应该是1");
        XCTAssertTrue([helper recordCountInTable:kTableName primaryKey:kColumnId condition:nil]==3, @"应该还剩下3条数据");
        
        XCTAssertTrue([helper remove:kTableName where:nil] == 3, @"发生变化的记录条数应该是3");
        XCTAssertTrue([helper recordCountInTable:kTableName primaryKey:kColumnId condition:nil]==0, @"应该还剩下0条数据");
    }
}

-(void)testDataTypes{
    // number part
    {
        NSDictionary* row = [self insertPersonWithAge:[NSNumber numberWithInt:1] height:[NSNumber numberWithFloat:180.10f]];
        XCTAssertTrue(((NSNumber*)row[kColumnAge]).intValue ==1);
        NSString* numberString = [NSString stringWithFormat:@"%.2f", ((NSNumber*)row[kColumnHeight]).floatValue];
        XCTAssertTrue([@"180.10" isEqualToString:numberString]);
        
        row = [self insertPersonWithAge:[NSNumber numberWithInteger:2] height:[NSNumber numberWithDouble:190.1]];
        XCTAssertTrue(((NSNumber*)row[kColumnAge]).intValue ==2);
        numberString = [NSString stringWithFormat:@"%.1f", ((NSNumber*)row[kColumnHeight]).floatValue];
        XCTAssertTrue([@"190.1" isEqualToString:numberString]);
        
        row = [self insertPersonWithAge:[NSNumber numberWithLong:3] height:[NSNumber numberWithDouble:190.1]];
        XCTAssertTrue(((NSNumber*)row[kColumnAge]).intValue ==3);
    }
    
    // blob part
    {
        NSData* avatarData = TData(@"imageData");
        NSDictionary* row = [self insertPersonWithAvatar:avatarData];
        NSData* fetchedData = row[kColumnAvatar];
        
        XCTAssertTrue(avatarData.hash == fetchedData.hash, @"原数据和query出来之后的数据哈希值应该一致");
    }
}

-(void)testTransactionInsert{
    QSQLiteOpenHelper* helper = TData(@"helper");
    XCTAssertTrue([helper recordCountInTable:kTableName primaryKey:kColumnId condition:nil]==0, @"应该啥数据都没的");
    
    NSError* error;
    [helper beginTransactionWithError:&error];
    XCTAssertNil(error, @"应该能正常启动存储过程");
    
    // insert a record
    NSMutableArray<QDBValue*>* contentValues = [[NSMutableArray alloc] init];
    [contentValues addObject:[QDBValue instanceForObject:@(1) withKey:kColumnAge]];
    long long recordId = [helper insert:kTableName contentValues:contentValues];
    XCTAssertTrue(recordId > 0);
    // rollback
    [helper rollbackTransaction];
    XCTAssertTrue([helper recordCountInTable:kTableName primaryKey:kColumnId condition:nil]==0, @"应该啥数据都没的");
    
    // start transaction again
    error = nil;
    [helper beginTransactionWithError:&error];
    XCTAssertNil(error, @"应该能正常启动存储过程");
    // insert a record
    contentValues = [[NSMutableArray alloc] init];
    [contentValues addObject:[QDBValue instanceForObject:@"Bob" withKey:kColumnName]];
    recordId = [helper insert:kTableName contentValues:contentValues];
    XCTAssertTrue(recordId > 0);
    [helper endTransaction];
    XCTAssertTrue([helper recordCountInTable:kTableName primaryKey:kColumnId condition:nil]== 1, @"应该有一条数据");

    NSString* where = [NSString stringWithFormat:@"%@=%lld", kColumnId, recordId];
    NSArray* rows = [helper query:kTableName columns:@[kColumnName] where:where];
    XCTAssertTrue(rows.count == 1);
    NSDictionary* result = rows.firstObject;
    XCTAssertTrue([result isKindOfClass:[NSDictionary class]]);
    XCTAssertTrue([@"Bob" isEqualToString:result[kColumnName]]);
}

-(void)testTransactionUpdate{
    QSQLiteOpenHelper* helper = TData(@"helper");
    XCTAssertTrue([helper recordCountInTable:kTableName primaryKey:kColumnId condition:nil]==0, @"应该啥数据都没的");
    
    // insert the value
    NSMutableArray<QDBValue*>* contentValues = [[NSMutableArray alloc] init];
    [contentValues addObject:[QDBValue instanceForObject:@"Cindy" withKey:kColumnName]];
    long long recordId = [helper insert:kTableName contentValues:contentValues];
    XCTAssertTrue(recordId > 0);
    XCTAssertTrue([helper recordCountInTable:kTableName primaryKey:kColumnId condition:nil]== 1, @"应该有一条数据");
    
    NSString* where = [NSString stringWithFormat:@"%@=%lld", kColumnId, recordId];
    
    NSError* error;
    [helper beginTransactionWithError:&error];
    XCTAssertNil(error, @"应该能正常启动存储过程");
    
    // update a record
    contentValues = [[NSMutableArray alloc] init];
    [contentValues addObject:[QDBValue instanceForObject:@"David" withKey:kColumnName]];
    
    
    XCTAssertTrue([helper update:kTableName contentValues:contentValues where:where] == 1);
    // rollback
    [helper rollbackTransaction];
    
    // confirm the record is not updated
    NSArray* rows = [helper query:kTableName columns:@[kColumnName] where:where];
    XCTAssertTrue(rows.count == 1);
    NSDictionary* result = rows.firstObject;
    XCTAssertTrue([result isKindOfClass:[NSDictionary class]]);
    XCTAssertTrue([@"Cindy" isEqualToString:result[kColumnName]]);
    
    
    // begin transaction again
    error = nil;
    [helper beginTransactionWithError:&error];
    XCTAssertNil(error, @"应该能正常启动存储过程");
    contentValues = [[NSMutableArray alloc] init];
    [contentValues addObject:[QDBValue instanceForObject:@"David" withKey:kColumnName]];
    // update the record
    XCTAssertTrue([helper update:kTableName contentValues:contentValues where:where] == 1);
    // commit change
    [helper endTransaction];
    
    // confirm the record is updated
    contentValues = [[NSMutableArray alloc] init];
    [contentValues addObject:[QDBValue instanceForObject:@"" withKey:kColumnName]];
    rows = [helper query:kTableName columns:@[kColumnName] where:where];
    XCTAssertTrue(rows.count == 1);
    result = rows.firstObject;
    XCTAssertTrue([result isKindOfClass:[NSDictionary class]]);
    XCTAssertTrue([@"David" isEqualToString:result[kColumnName]]);
}


-(void)testTransactionDelete{
    QSQLiteOpenHelper* helper = TData(@"helper");
    XCTAssertTrue([helper recordCountInTable:kTableName primaryKey:kColumnId condition:nil]==0, @"应该啥数据都没的");
    
    // insert the value
    NSMutableArray<QDBValue*>* contentValues = [[NSMutableArray alloc] init];
    [contentValues addObject:[QDBValue instanceForObject:@"Cindy" withKey:kColumnName]];
    long long recordId = [helper insert:kTableName contentValues:contentValues];
    XCTAssertTrue(recordId > 0);
    XCTAssertTrue([helper recordCountInTable:kTableName primaryKey:kColumnId condition:nil]== 1, @"应该有一条数据");
    
    NSString* where = [NSString stringWithFormat:@"%@=%lld", kColumnId, recordId];
    
    NSError* error;
    [helper beginTransactionWithError:&error];
    XCTAssertNil(error, @"应该能正常启动存储过程");
    XCTAssertTrue([helper remove:kTableName where:where] == 1);
    // rollback
    [helper rollbackTransaction];
    
    // confirm the record is not updated
    NSArray* rows = [helper query:kTableName columns:@[kColumnName] where:where];
    XCTAssertTrue(rows.count == 1);
    NSDictionary* result = rows.firstObject;
    XCTAssertTrue([result isKindOfClass:[NSDictionary class]]);
    XCTAssertTrue([@"Cindy" isEqualToString:result[kColumnName]]);
    
    
    // begin transaction again
    error = nil;
    [helper beginTransactionWithError:&error];
    XCTAssertNil(error, @"应该能正常启动存储过程");
    XCTAssertTrue([helper remove:kTableName where:where] == 1);
    // commit change
    [helper endTransaction];
    
    // confirm the record is updated
    XCTAssertTrue([helper recordCountInTable:kTableName primaryKey:kColumnId condition:where] == 0);
}

-(void)testTransactionConflict{
    QSQLiteOpenHelper* helper = TData(@"helper");
    
    // insert the value
    NSMutableArray<QDBValue*>* contentValues = [[NSMutableArray alloc] init];
    [contentValues addObject:[QDBValue instanceForObject:@"Cindy" withKey:kColumnName]];
    long long recordId = [helper insert:kTableName contentValues:contentValues];
    XCTAssertTrue(recordId > 0);
    XCTAssertTrue([helper recordCountInTable:kTableName primaryKey:kColumnId condition:nil]== 1, @"应该有一条数据");
    
    NSString* where = [NSString stringWithFormat:@"%@=%lld", kColumnId, recordId];
    
    
    NSError* error;
    [helper beginTransactionWithError:&error];
    XCTAssertNil(error, @"应该能正常启动存储过程");
    contentValues = [[NSMutableArray alloc] init];
    [contentValues addObject:[QDBValue instanceForObject:@"Cindy" withKey:kColumnName]];
    recordId = [helper insert:kTableName contentValues:contentValues];
    where = [NSString stringWithFormat:@"%@=%lld", kColumnId, recordId];
    
    XCTAssertTrue([helper recordCountInTable:kTableName primaryKey:kColumnId condition:where]== 1, @"应该有一条数据");
    
    [helper rollbackTransaction];
    XCTAssertTrue([helper recordCountInTable:kTableName primaryKey:kColumnId condition:where]== 0, @"应该没有数据");
}

//// 0.1978s
//- (void)testTransactionPerformance {
//    QSQLiteOpenHelper* helper = TData(@"helper");
//    XCTAssertTrue([helper recordCountInTable:kTableName primaryKey:kColumnId condition:nil] == 0);
//    
//    NSMutableArray* contentValues = [[NSMutableArray alloc] init];
//    NSDate* currentDate = [NSDate date];
//    [helper beginTransactionWithError:nil];
//    for(int i=0; i< 10000; i++){
//        [contentValues removeAllObjects];
//        [contentValues addObject:[QDBValue instanceForObject:@(i) withKey:kColumnAge]];
//        [helper insert:kTableName contentValues:contentValues];
//    }
//    [helper endTransaction];
//    NSLog(@"Seconds: %f.2", [NSDate date].timeIntervalSince1970 - currentDate.timeIntervalSince1970);
//    
//    long recordCount = [helper recordCountInTable:kTableName primaryKey:kColumnId condition:nil];
//    NSLog(@"Record count: %ld", recordCount);
//    XCTAssertTrue(recordCount == 10000, @"应该有一万份记录");
//}
//
//// 36.05s
//- (void)testNormalPerformance {
//    QSQLiteOpenHelper* helper = TData(@"helper");
//    NSMutableArray* contentValues = [[NSMutableArray alloc] init];
//    NSDate* currentDate = [NSDate date];
//    
//    for(int i=0; i< 10000; i++){
//        [contentValues removeAllObjects];
//        [contentValues addObject:[QDBValue instanceForObject:@(i) withKey:kColumnAge]];
//        [helper insert:kTableName contentValues:contentValues];
//    }
//    
//    NSLog(@"Seconds: %f.2", [NSDate date].timeIntervalSince1970 - currentDate.timeIntervalSince1970);
//    long recordCount = [helper recordCountInTable:kTableName primaryKey:kColumnId condition:nil];
//    NSLog(@"Record count: %ld", recordCount);
//    XCTAssertTrue(recordCount == 10000, @"应该有一万份记录");
//}

#pragma mark - toolkits
-(NSDictionary*)insertPersonWithAge:(NSNumber*)age height:(NSNumber*)height{
    QSQLiteOpenHelper* helper = TData(@"helper");
    NSMutableArray<QDBValue*>* contentValues = [[NSMutableArray alloc] init];
    [contentValues addObject:[QDBValue instanceForObject:age withKey:kColumnAge]];
    [contentValues addObject:[QDBValue instanceForObject:height withKey:kColumnHeight]];
    long long recordId = [helper insert:kTableName contentValues:contentValues];
    XCTAssertTrue( recordId > 0);
    
    NSString* where = [NSString stringWithFormat:@"%@=%lld", kColumnId, recordId];
    NSArray* rows = [helper query:kTableName columns:@[kColumnAge, kColumnHeight] where:where];
    XCTAssertTrue(rows.count == 1);
    NSDictionary* result = rows.firstObject;
    XCTAssertTrue([result isKindOfClass:[NSDictionary class]]);
    return result;
}

-(NSDictionary*)insertPersonWithAvatar:(NSData*)imageData{
    QSQLiteOpenHelper* helper = TData(@"helper");
    NSMutableArray<QDBValue*>* contentValues = [[NSMutableArray alloc] init];
    [contentValues addObject:[QDBValue instanceForObject:imageData withKey:kColumnAvatar]];
    long long recordId = [helper insert:kTableName contentValues:contentValues];
    XCTAssertTrue( recordId > 0);
    
    NSString* where = [NSString stringWithFormat:@"%@=%lld", kColumnId, recordId];
    NSArray* rows = [helper query:kTableName columns:@[kColumnAvatar] where:where];
    XCTAssertTrue(rows.count == 1);
    NSDictionary* result = rows.firstObject;
    XCTAssertTrue([result isKindOfClass:[NSDictionary class]]);
    return result;
}

#pragma mark - db delegate
-(NSString*) pathToCopyBundleDBFileForSQLiteOpenHelper:(QSQLiteOpenHelper *)openHelper
                                              withName:(NSString*)name{
    NSBundle* bundle = [NSBundle bundleForClass:[self class]];
    return [bundle pathForResource:TData(@"dbName") ofType:nil];
}
@end