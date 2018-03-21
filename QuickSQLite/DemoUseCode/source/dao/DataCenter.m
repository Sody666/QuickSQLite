//
//  DataCenter.m
//  Quick SQLite Demo
//
//  Created by sudi on 2016/12/20.
//  Copyright © 2016年 quick. All rights reserved.
//

#import "DataCenter.h"

#define kTableName  @"person"

#define kColumnId       @"_id"
#define kColumnName     @"name"
#define kColumnAge      @"age"
#define kColumnAvatar   @"avatar"
#define kColumnHeight   @"height"


@interface DataCenter()<QSQLiteOpenHelperDelegate>
@property (nonatomic, strong) QSQLiteOpenHelper* dbHelper;
@end

@implementation DataCenter

+(id)defaultDataCenter{return nil;}
-(NSString*)databaseName{return nil;}

-(id)init{
    self = [super init];
    if (self) {
        NSString* key = [self keyword];
        _dbHelper = [[QSQLiteOpenHelper alloc] initWithName:[self databaseName] key:key version:1 pageSize:[self pageSize] openDelegate:self];// clear version
    }
    return self;
}

-(NSUInteger)pageSize{
    return QDBPageSizeDefault;
}

-(NSString*)keyword{
    return nil;
}

-(void)test_saveDefaultPersonForCount:(NSInteger)count{
    NSDictionary* values = @{
                             kColumnName:@"Alice",
                             kColumnAge:@(21),
                             kColumnHeight:@(178),
                             };
    for(int i=0; i<count; i++){
        [self.dbHelper insert:kTableName values:values];
    }
}

-(void)test_saveTransactionDefaultPersonForCount:(NSInteger)count{
    NSDictionary* values = @{
                             kColumnName:@"Alice",
                             kColumnAge:@(21),
                             kColumnHeight:@(178),
                             };
    [self.dbHelper beginTransactionWithError:nil];
    for(int i=0; i<count; i++){
        [self.dbHelper insert:kTableName values:values];
    }
    [self.dbHelper endTransaction];
}

-(BOOL)savePerson:(Person*)person{
    NSDictionary* values = @{
                             kColumnName: person.name,
                             kColumnAvatar: UIImagePNGRepresentation(person.avatar),
                             kColumnAge: @(person.age),
                             kColumnHeight: @(person.height)
                             };
    
    if(person.identity == 0){
        person.identity = (NSUInteger)[self.dbHelper insert:kTableName values:values];
        return person.identity > 0;
    }else{
        NSString* where = [NSString stringWithFormat:@"%@=%ld", kColumnId, (unsigned long)person.identity];
        return [self.dbHelper update:kTableName values:values where:where] > 0;
    }
}

-(NSArray*)allPersons{
    NSArray* rows = [self.dbHelper query:kTableName columns:@[kColumnId, kColumnAvatar, kColumnHeight, kColumnAge, kColumnName] where:nil];
    
    NSMutableArray* result = [[NSMutableArray alloc] init];
    for (NSDictionary* row in rows) {
        Person* person = [[Person alloc] init];
        person.name = row[kColumnName];
        person.age = ((NSNumber*)row[kColumnAge]).unsignedIntegerValue;
        person.height =((NSNumber*)row[kColumnHeight]).floatValue;
        person.identity = ((NSNumber*)row[kColumnId]).unsignedIntegerValue;
        if(!isNull(row[kColumnAvatar])){
            person.avatar = [UIImage imageWithData:row[kColumnAvatar]];
        }
        
        [result addObject:person];
    }
    
    return result;
}

#pragma mark - db open delegate
-(NSString*) pathToCopyBundleDBFileForSQLiteOpenHelper:(QSQLiteOpenHelper *)openHelper
                                              withName:(NSString*)name{
    NSLog(@"Claiming: %@:%@:%@", name, [self databaseName], [[NSBundle mainBundle] pathForResource:[self databaseName] ofType:nil]);
    return [[NSBundle mainBundle] pathForResource:[self databaseName] ofType:nil];
}
@end
