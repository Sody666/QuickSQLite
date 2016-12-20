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
 Get an instance with object and key.
 Note: object type should be either NSData or NSString or NSNumber
 Example:
 instanceForObject:@(11) withKey:@"age"
 instanceForObject:@"" withKey:@"name"

 @param object object to be used. Provide an example object if querying.
 @param key key for the content
 @return instance initialized
 */
+(QDBValue*)instanceForObject:(id)object withKey:(const NSString*)key;


/**
 Get instance with key.
 This should be used while querying data.
 Data type will be determined when data is fetched.

 @param key key for the content
 @return instance initialized
 */
+(QDBValue*)instanceWithKey:(const NSString*)key;
/**
 *  After query, get the unbinded data.
 *  The type of it is the same as the one in instanceForObject:withKey:
 *  Note: for other actions such as update, insert, the result is the same
 *        as the original.
 *
 *  @return content data
 */
-(id)value;


/**
 Prepare an array of values with a dictionary

 @param keyValues input key value pair
 @return prepared values
 */
+(NSArray*)valuesWithDictionary:(const NSDictionary*)keyValues;

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
+(void)mappingContentValues:(const NSArray<QDBValue*>*)contentValues
                      query:(NSString**)queryOutput
                     update:(NSString**)updateOutput
                     insert:(NSString**)insertOutput;

#pragma mark - binding and unbinding data with statement
/**
 *  Bind values from statement.
 *
 *  @param values value to bind to
 *  @param stmt   value to bind from
 *  @return whether binding is done.
 */
+(BOOL)unbindRowIntoValues:(const NSArray<QDBValue*> *)values
             fromStatement:(sqlite3_stmt *)stmt;

/**
 *  Bind values from statement.
 *
 *  @param values value to bind to
 *  @param stmt   value to bind from
 *  @return whether row of data. nil if failed.
 */
+(NSDictionary*)unbindRowIntoDictionaryWithValues:(const NSArray<QDBValue*> *)values
                                    fromStatement:(sqlite3_stmt *)stmt;


/**
 *  Bind values from statement.
 *
 *  @param values value to bind to
 *  @param stmt   value to bind from
 */
+(void)bindRowWithValues:(const NSArray *)values intoStatement:(sqlite3_stmt *)stmt;


@end
