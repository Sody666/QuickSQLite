//
//  QuickSqliteException.m
//  QuickSQLite
//
//  Created by sudi on 2016/12/28.
//
//

#import "QDBException.h"

@implementation QDBException
+(id)exceptionForReason:(NSString*)reason userInfo:(NSDictionary*)userInfo{
    return [self exceptionWithName:@"QuickSQLite Framework Exception" reason:reason userInfo:userInfo];
}
@end
