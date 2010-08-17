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

#ifndef __CLASS_SCOPE_H__
#define __CLASS_SCOPE_H__

#include <compiler/analysis/block_scope.h>
#include <compiler/analysis/function_container.h>
#include <compiler/statement/class_statement.h>
#include <util/json.h>
#include <util/case_insensitive.h>
#include <compiler/option.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

DECLARE_BOOST_TYPES(StatementList);
DECLARE_BOOST_TYPES(FunctionScope);
DECLARE_BOOST_TYPES(ClassScope);
DECLARE_BOOST_TYPES(FileScope);

/**
 * A class scope corresponds to a class declaration. We store all
 * inferred types and analyzed results here, so not to pollute syntax trees.
 */
class ClassScope : public BlockScope, public FunctionContainer,
                   public JSON::ISerializable {

public:
  enum KindOf {
    KindOfObjectClass,
    KindOfAbstractClass,
    KindOfFinalClass,
    KindOfInterface,
  };

  enum Attribute {
    HasConstructor = 1, // set iff there is a __construct method.
                        // check classNameConstructor if you want to know
                        // whether there is a class-name constructor.
    HasDestructor  = 2,
    HasUnknownMethodHandler = 4,
    System = 8,
    Extension = 16,
    classNameConstructor = 32,
    HasUnknownPropGetter = 64,
    HasUnknownPropSetter = 128
  };
  enum Modifier {
    Public = 1,
    Protected = 2,
    Private = 4,
    Static = 8,
    Abstract = 16,
    Final = 32
  };
  enum Derivation {
    FromNormal = 0,
    DirectFromRedeclared,
    IndirectFromRedeclared
  };

  enum JumpTableName {
    JumpTableInvoke,
    JumpTableStaticInvoke
  };

public:
  ClassScope(KindOf kindOf, const std::string &name,
             const std::string &parent,
             const std::vector<std::string> &bases,
             const std::string &docComment, StatementPtr stmt,
             FileScopePtr file);


  /**
   * Special constructor for extension classes.
   */
  ClassScope(AnalysisResultPtr ar,
             const std::string &name, const std::string &parent,
             const std::vector<std::string> &bases,
             const FunctionScopePtrVec &methods);

  bool classNameCtor() const {
    return getAttribute(classNameConstructor);
  }
  const std::string &getOriginalName() const;


  virtual std::string getId(CodeGenerator &cg) const;

  void checkDerivation(AnalysisResultPtr ar, hphp_string_set &seen);
  const std::string &getParent() const { return m_parent;}
  std::string getHeaderFilename(CodeGenerator &cg);

  /**
   * Returns topmost parent class that has the method.
   */
  ClassScopePtr getRootParent(AnalysisResultPtr ar,
                              const std::string &methodName = "");
  void getRootParents(AnalysisResultPtr ar, const std::string &methodName,
                      ClassScopePtrVec &roots, ClassScopePtr curClass);

  /**
   * Whether this is a user-defined class.
   */
  bool isUserClass() const { return !getAttribute(System);}
  bool isExtensionClass() const { return getAttribute(Extension); }
  bool isDynamic() const { return m_dynamic; }
  bool isBaseClass() const { return m_bases.empty(); }

  /**
   * Whether this class name was declared twice or more.
   */
  void setRedeclaring(AnalysisResultPtr ar, int redecId);
  bool isRedeclaring() const { return m_redeclaring >= 0;}
  int getRedeclaringId() { return m_redeclaring; }

  void setStaticDynamic(AnalysisResultPtr ar);
  void setDynamic(AnalysisResultPtr ar, const std::string &name);

  /* For class_exists */
  void setVolatile() { m_volatile = true;}
  bool isVolatile() { return m_volatile;}

  /* For code generation of os_static_initializer */
  void setNeedStaticInitializer() { m_needStaticInitializer = true;}
  bool needStaticInitializer() { return m_needStaticInitializer;}

  bool needLazyStaticInitializer();

  Derivation derivesFromRedeclaring() const {
    return m_derivesFromRedeclaring;
  }

  /* Whether this class is brought in by a separable extension */
  void setSepExtension() { m_sep = true;}
  bool isSepExtension() const { return m_sep;}

  /**
   * Get/set attributes.
   */
  void setSystem();
  void setAttribute(Attribute attr) { m_attribute |= attr;}
  void clearAttribute(Attribute attr) { m_attribute &= ~attr;}
  bool getAttribute(Attribute attr) const {
    return m_attribute & attr;
  }
  bool hasAttribute(Attribute attr,
                    AnalysisResultPtr ar) const; // recursive

  void addMissingMethod(const std::string &name) {
    m_missingMethods.insert(name);
  }

  /**
   * Called by ClassScope to prepare name => method/property map.
   */
  void collectMethods(AnalysisResultPtr ar,
                      StringToFunctionScopePtrMap &func,
                      bool collectPrivate = true,
                      bool forInvoke = false);

  /**
   * Whether or not we can directly call c_ObjectData::o_invoke() when lookup
   * in this class fails. If false, we need to call parent::o_invoke(), which
   * may be redeclared or may have private methods that need to check class
   * context.
   */
  bool needsInvokeParent(AnalysisResultPtr ar, bool considerSelf = true);

  /*
    void collectProperties(AnalysisResultPtr ar,
    std::set<std::string> &names,
    bool collectPrivate = true) const;

  */
  /**
   * Testing whether this class derives from another.
   */
  bool derivesDirectlyFrom(AnalysisResultPtr ar,
                           const std::string &base) const;
  bool derivesFrom(AnalysisResultPtr ar, const std::string &base,
                   bool strict, bool def) const;

 /**
  * Find a common parent of two classes; returns "" if there is no such.
  */
  static std::string findCommonParent(AnalysisResultPtr ar,
                                      const std::string cn1,
                                      const std::string cn2);

  /**
   * Look up function by name.
   */
  FunctionScopePtr findFunction(AnalysisResultPtr ar,
                                const std::string &name,
                                bool recursive,
                                bool exclIntfBase = false);

  /**
   * Look up constructor, both __construct and class-name constructor.
   */
  FunctionScopePtr findConstructor(AnalysisResultPtr ar,
                                   bool recursive);

  TypePtr checkProperty(const std::string &name, TypePtr type,
                        bool coerce, AnalysisResultPtr ar,
                        ConstructPtr construct, int &properties);
  TypePtr checkStatic(const std::string &name, TypePtr type,
                      bool coerce, AnalysisResultPtr ar,
                      ConstructPtr construct, int &properties);
  TypePtr checkConst(const std::string &name, TypePtr type,
                     bool coerce, AnalysisResultPtr ar,
                     ConstructPtr construct,
                     const std::vector<std::string> &bases,
                     BlockScope *&defScope);

  /**
   * Collect parent class names.
   */
  void getAllParents(AnalysisResultPtr ar,
                     std::vector<std::string> &names) {
    if (m_stmt) {
      if (isInterface()) {
        boost::dynamic_pointer_cast<InterfaceStatement>
          (m_stmt)->getAllParents(ar, names);
      } else {
        boost::dynamic_pointer_cast<ClassStatement>
          (m_stmt)->getAllParents(ar, names);
      }
    }
  }

  std::vector<std::string> &getBases() { return m_bases;}

  FileScopePtr getFileScope() {
    FileScopePtr fs = m_file.lock();
    return fs;
  }

  ClassScopePtr getParentScope(AnalysisResultPtr ar);

  /**
   * Output class meta info for g_class_map.
   */
  void outputCPPClassMap(CodeGenerator &cg, AnalysisResultPtr ar);

  /**
   * Serialize the iface, not everything.
   */
  void serialize(JSON::OutputStream &out) const;

  static void outputCPPClassVarInitImpl(
    CodeGenerator &cg, const StringToClassScopePtrVecMap &classScopes,
    const std::vector<const char *> &classes);

  void outputCPPDynamicClassDecl(CodeGenerator &cg);
  void outputCPPDynamicClassImpl(CodeGenerator &cg, AnalysisResultPtr ar);
  static void outputCPPDynamicClassCreateDecl(CodeGenerator &cg);
  static void outputCPPDynamicClassCreateImpl
  (CodeGenerator &cg, const StringToClassScopePtrVecMap &classScopes,
   const std::vector<const char*> &classes);
  static void outputCPPInvokeStaticMethodImpl
  (CodeGenerator &cg, const StringToClassScopePtrVecMap &classScopes,
   const std::vector<const char*> &classes);
  static void outputCPPGetCallInfoStaticMethodImpl
  (CodeGenerator &cg, const StringToClassScopePtrVecMap &classScopes,
   const std::vector<const char*> &classes);
  static void outputCPPGetStaticPropertyImpl
  (CodeGenerator &cg, const StringToClassScopePtrVecMap &classScopes,
   const std::vector<const char*> &classes);
  static void outputCPPGetClassConstantImpl
  (CodeGenerator &cg, const StringToClassScopePtrVecMap &classScopes,
   const std::vector<const char*> &classes);
  void outputCPPStaticInitializerDecl(CodeGenerator &cg);
  bool isInterface() { return m_kindOf == KindOfInterface; }
  bool isFinal() { return m_kindOf == KindOfFinalClass; }
  bool isAbstract() { return m_kindOf == KindOfAbstractClass; }
  bool hasProperty(const std::string &name);
  bool hasConst(const std::string &name);
  void outputCPPHeader(CodeGenerator &cg, AnalysisResultPtr ar,
                       CodeGenerator::Output output);
  void outputCPPJumpTableDecl(CodeGenerator &cg, AnalysisResultPtr ar);

  void outputMethodWrappers(CodeGenerator &cg, AnalysisResultPtr ar);

  /**
   * This prints out all the support methods (invoke, create, destructor,
   * etc.)
   * This is here instead of ClassStatement because I want to be able to call
   * these for extension classes that don't have a statement.
   */
  void outputCPPSupportMethodsImpl(CodeGenerator &cg, AnalysisResultPtr ar);

  /**
   * These output wrappers for class methods so that class definitions don't
   * have to be used in generating global jump tables.
   */
  void outputCPPGlobalTableWrappersDecl(CodeGenerator &cg,
                                        AnalysisResultPtr ar);
  void outputCPPGlobalTableWrappersImpl(CodeGenerator &cg,
                                        AnalysisResultPtr ar);

  /*
    returns 1 if it does, -1 if it may, and 0 if it doesnt
  */
  int implementsArrayAccess(AnalysisResultPtr ar);
  /*
    returns 1 if it does, -1 if it may, and 0 if it doesnt
  */
  int implementsAccessor(AnalysisResultPtr ar, const char *name);


  void clearBases() {
    m_bases.clear();
    m_parent = "";
  }

  void outputCPPStaticMethodWrappers(CodeGenerator &cg, AnalysisResultPtr ar,
                                     std::set<std::string> &done,
                                     const char *cls);
  /**
   * Override function container
   */
  virtual bool addFunction(AnalysisResultPtr ar, FunctionScopePtr funcScope);

  enum TableType {
    Invoke,
    Eval,
    CallInfo
  };
  void outputCPPJumpTable(CodeGenerator &cg, AnalysisResultPtr ar,
                          bool staticOnly, bool dynamicObject = false,
                          TableType type = Invoke);

  void outputVolatileCheckBegin(CodeGenerator &cg,
                                AnalysisResultPtr ar,
                                const std::string &name);
  void outputVolatileCheckEnd(CodeGenerator &cg);
  static void OutputVolatileCheckBegin(CodeGenerator &cg,
                                       AnalysisResultPtr ar,
                                       const std::string &origName);
  static void OutputVolatileCheckEnd(CodeGenerator &cg);
  static void OutputVolatileCheck(CodeGenerator &cg, AnalysisResultPtr ar,
      const std::string &origName, bool noThrow);


  /**
   * Whether or not the specified jump table is empty.
   */
  bool hasAllJumpTables() const {
    return m_emptyJumpTables.empty();
  }
  bool hasJumpTable(JumpTableName name) const {
    return m_emptyJumpTables.find(name) == m_emptyJumpTables.end();
  }

protected:
  void findJumpTableMethods(CodeGenerator &cg, AnalysisResultPtr ar,
                            bool staticOnly, std::vector<const char *> &funcs);
private:
  // need to maintain declaration order for ClassInfo map
  FunctionScopePtrVec m_functionsVec;

  FileScopeWeakPtr m_file;
  KindOf m_kindOf;
  std::string m_parent;
  mutable std::vector<std::string> m_bases;
  mutable int m_attribute;
  bool m_dynamic;
  int m_redeclaring; // multiple definition of the same class
  bool m_volatile; // for class_exists
  bool m_needStaticInitializer; // for os_static_initializer
  Derivation m_derivesFromRedeclaring;
  std::set<std::string> m_missingMethods;
  bool m_sep;

  std::set<JumpTableName> m_emptyJumpTables;

  static void outputCPPClassJumpTable
  (CodeGenerator &cg, const StringToClassScopePtrVecMap &classScopes,
   const std::vector<const char*> &classes, const char* macro);
  void outputCPPMethodInvokeTable
    (CodeGenerator &cg, AnalysisResultPtr ar,
     const std::vector <const char*> &keys,
     const StringToFunctionScopePtrVecMap &funcScopes, bool fewArgs,
     bool staticOnly, TableType type);
  void outputCPPMethodInvokeTableSupport(CodeGenerator &cg,
      AnalysisResultPtr ar, const std::vector<const char*> &keys,
      const StringToFunctionScopePtrVecMap &funcScopes, bool fewArgs);
  hphp_const_char_imap<int> m_implemented;
};

///////////////////////////////////////////////////////////////////////////////
}
#endif // __CLASS_SCOPE_H__
