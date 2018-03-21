//
//  QuickSqliteException.h
//  QuickSQLite
//
//  Created by sudi on 2016/12/28.
//
//

#import <Foundation/Foundation.h>

@interface QDBException : NSException
+(id)exceptionForReason:(NSString*)reason userInfo:(NSDictionary*)userInfo;
@end
