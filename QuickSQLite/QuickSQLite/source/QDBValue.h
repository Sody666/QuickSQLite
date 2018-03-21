//
//  CTContentValue.h
//  SqliteDemo
//
//  Created by Sou Dai on 13-5-29.
//

#import <Foundation/Foundation.h>
#import <sqlite3.h>



@interface QDBValue : NSObject

/**
 Get the value of the DBValue.
 type of the value:
 TEXT:      NSString
 REAL:      NSNumber
 BLOB:      NSData
 INTEGER:   NSNumber
 
 @return the value as an object
 */
-(id)value;

#pragma mark - binding and unbinding data with statement
/**
 *  unbind values from statement.
 *  For large set result use.
 *
 *  @param values value to bind to
 *  @param stmt   statement to bind from
 *  @return whether binding is done.
 */
+(BOOL)unbindRowIntoValues:(const NSArray<QDBValue*> *)values
             fromStatement:(sqlite3_stmt *)stmt;
@end
