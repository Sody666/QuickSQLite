//
//  QLayoutResult.h
//  LibSourceUser
//
//  Created by 苏第 on 17/11/5.
//  Copyright © 2017年 Su Di. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

@interface QLayoutResult : NSObject
@property (nonatomic, strong) NSDictionary* createdViews;
@property (nonatomic, strong) NSDictionary* viewsData;
@property (nonatomic, strong) NSDictionary* namedConstraints;
/**
 *  Get the view with name.
 **/
-(id)viewNamed:(NSString*)name;

/**
 *  Get the data for view with name.
 **/
-(id)dataForViewNamed:(NSString*)name;

/**
 *  Get the named constraint.
 **/
-(NSLayoutConstraint*)constraintNamed:(NSString*)name;
@end
