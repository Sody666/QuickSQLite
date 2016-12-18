//
//  DataCenter.m
//  QuickSQLiteDemo
//
//  Created by sudi on 2016/12/16.
//  Copyright © 2016年 quick. All rights reserved.
//

#import "DataCenter.h"
#import "QSQLite.h"
#import "PersonModel.h"

#define kTableName @"person"

#define kColumnId   @"_id"
#define kColumnAge  @"age"
#define kColumnName @"name"

@interface DataCenter()<QSQLiteOpenHelperDelegate>
@property (nonatomic, strong) QSQLiteOpenHelper* dbHelper;
@end

@implementation DataCenter
-(id)init{
    self=[super init];
    if (self) {
        _dbHelper = [[QSQLiteOpenHelper alloc] initWithName:@"demo" version:1 openDelegate:self];
    }
    
    return self;
}

-(NSArray*)allPersonAvailable{
    NSMutableArray* contentValues = [[NSMutableArray alloc] init];
    [contentValues addObject:[QDBValue instanceForObject:@"" withKey:kColumnName]];
    [contentValues addObject:[QDBValue instanceForObject:@(1) withKey:kColumnAge]];
    [contentValues addObject:[QDBValue instanceForObject:@(1) withKey:kColumnId]];
    
    NSMutableArray* result = [[NSMutableArray alloc] init];
    sqlite3_stmt* statement;
    
    if([self.dbHelper query:kTableName
                 columns:contentValues
                   where:nil
                 orderBy:kColumnAge
                   limit:nil
                 groupBy:nil
                  statement:&statement]){
        
        while ([QDBValue unbindRowIntoValues:contentValues fromStatement:statement]) {
            PersonModel* person = [[PersonModel alloc] init];
            person.name = [contentValues[0] value];
            person.age = [[contentValues[1] value] intValue];
            person.identity = [[contentValues[2] value] longValue];
            [result addObject:person];
        }
        
        sqlite3_finalize(statement);
    }
    
    return result;
}

-(void)upsertPerson:(PersonModel*)person{
    NSMutableArray* contentValues = [[NSMutableArray alloc] init];
    [contentValues addObject:[QDBValue instanceForObject:person.name withKey:kColumnName]];
    [contentValues addObject:[QDBValue instanceForObject:@(person.age) withKey:kColumnAge]];
    
    if(person.identity > 0){
        NSString* where = [NSString stringWithFormat:@"%@=%d", kColumnId, person.identity];
        if([self.dbHelper isRecordAvailableInTable:kTableName
                                        primaryKey:kColumnId
                                         condition:where]){
                                             [self.dbHelper update:kTableName contentValues:contentValues where:where];
            return;
        }else{
            [contentValues addObject:[QDBValue instanceForObject:@(person.identity) withKey:kColumnId]];
        }
    }
    
    [self.dbHelper insert:kTableName contentValues:contentValues];
}

#pragma mark - helper delegate
// will copy bundle db to overwrite sandbox as soon as db version upgraded
-(NSString*) pathToCopyBundleDBFileForSQLiteOpenHelper:(QSQLiteOpenHelper *)openHelper
                                              withName:(NSString*)name{
    if([@"demo" isEqualToString:name]){
        return [[NSBundle mainBundle] pathForResource:@"demo" ofType:@"sql"];
    }
    
    return nil;
}
@end
