//
//  DataCenter.h
//  QuickSQLiteDemo
//
//  Created by sudi on 2016/12/16.
//  Copyright © 2016年 quick. All rights reserved.
//

#import <Foundation/Foundation.h>

@class PersonModel;
@interface DataCenter : NSObject
-(NSArray*)allPersonAvailable;
-(void)upsertPerson:(PersonModel*)person;
@end
