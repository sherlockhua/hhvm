/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/
// @generated by HipHop Compiler

#ifndef __GENERATED_php_classes_iterator_h__
#define __GENERATED_php_classes_iterator_h__

#include <runtime/base/hphp_system.h>
#include <php/classes/iterator.fw.h>

// Declarations
#include <cls/OuterIterator.h>
#include <cls/Serializable.h>
#include <cls/Traversable.h>
#include <cls/Countable.h>
#include <cls/ArrayIterator.h>
#include <cls/Iterator.h>
#include <cls/AppendIterator.h>
#include <cls/SeekableIterator.h>
#include <cls/RecursiveIterator.h>
#include <cls/RecursiveDirectoryIterator.h>
#include <cls/DirectoryIterator.h>
#include <cls/IteratorAggregate.h>
#include <cls/RecursiveIteratorIterator.h>
#include <cls/FilterIterator.h>
#include <php/classes/arrayaccess.h>
#include <php/classes/splfile.h>
#include <php/classes/exception.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// Includes and Functions
Variant pm_php$classes$iterator_php(bool incOnce = false, LVariableTable* variables = NULL, Globals *globals = get_globals());

// Redeclared Functions

// Dynamic Class Declarations
Object co_ArrayIterator(CArrRef params, bool init = true);
Object coo_ArrayIterator();
Object co_AppendIterator(CArrRef params, bool init = true);
Object coo_AppendIterator();
Object co_RecursiveDirectoryIterator(CArrRef params, bool init = true);
Object coo_RecursiveDirectoryIterator();
Object co_DirectoryIterator(CArrRef params, bool init = true);
Object coo_DirectoryIterator();
Object co_RecursiveIteratorIterator(CArrRef params, bool init = true);
Object coo_RecursiveIteratorIterator();
Object co_FilterIterator(CArrRef params, bool init = true);
Object coo_FilterIterator();

///////////////////////////////////////////////////////////////////////////////
}

#endif // __GENERATED_php_classes_iterator_h__
