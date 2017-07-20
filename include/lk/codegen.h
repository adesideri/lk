#ifndef __lk_codegen_h
#define __lk_codegen_h

#include <lk/absyn.h>
#include <lk/vm.h>

namespace lk {

/** Codegen produces bytecode from a tree of nodes.
* \class codegen
*
* Uses root node, a list_t node containing a vector of statements to produce an executable
* stack and lists of identifiers for interpretation by virtual machine. Original LK script 
* text stored in srcpos_t are transfered to instr's, which are pushed onto stack. 
*
* Labels created at beginning of compound nodes (such as for loops or conditionals) contain
* information about position in stack of starting instruction. 
*/
class codegen
{
public:
	codegen();
	
	lk_string error() { return m_errStr; }
	
	/// traverses tree and identifes node types to create instructions, variables, data structures, labels, etc
	bool generate( lk::node_t *root );
	/// copies labels, constants, & identifers into bytecode
	size_t get( bytecode &b );
	/// writes the bytecode into assembly
	void textout( lk_string &assembly, lk_string &bytecode );

private:	

/** Composes codegen's stack.
* \struct instr
*
* Makes up the instructions that go into the m_asm stack by containing the source data
* position, the operation to be done, and any flags
*
*/
	struct instr {
		instr( srcpos_t sp, Opcode _op, int _arg, const char *lbl = 0 )
			: pos(sp), op(_op), arg(_arg) {
			label = 0;
			if ( lbl ) label = new lk_string(lbl);
		}

		instr( const instr &cpy )
		{
			copy( cpy );
		}
		~instr() { 
			if (label) delete label;
		}
		void copy( const instr &cpy )
		{
			pos = cpy.pos;
			op = cpy.op;
			arg = cpy.arg;
			label = 0;
			if ( cpy.label )
				label = new lk_string(*cpy.label);
		}

		instr &operator=( const instr &rhs ) {
			copy( rhs );
			return *this;
		}

		lk::srcpos_t pos;
		Opcode op;
		int arg;
		lk_string *label;
	};

	/// functions as virtual stack
	std::vector<instr> m_asm;
	/// maps from label # to position in stack, ie: the fifth label created label, "L5", is in m_asm position 1
	typedef unordered_map< lk_string, int, lk_string_hash, lk_string_equal > LabelMap;
	LabelMap m_labelAddr;
	std::vector< vardata_t > m_constData;
	std::vector< lk_string > m_idList;
	int m_labelCounter;
	/// stores labels associated with loops: continueAddr for advancing loops, break for end
	std::vector<lk_string> m_breakAddr, m_continueAddr;
	lk_string m_errStr;

	bool error( const char *fmt, ... );
	bool error( const lk_string &s );

	int place_identifier( const lk_string &id );
	int place_const( vardata_t &d );
	int const_value( double value );
	int const_literal( const lk_string &lit );
	lk_string new_label();
	void place_label( const lk_string &s );
	int emit( srcpos_t pos, Opcode o, int arg = 0);						///< makes instructions & adds to m_asm
	int emit( srcpos_t pos, Opcode o, const lk_string &L );
	bool initialize_const_vec( lk::list_t *v, vardata_t &vvec );		///< creates vector vardata type
	bool initialize_const_hash( lk::list_t *v, vardata_t &vhash );		///< creates hash vardata type
	bool pfgen_stmt( lk::node_t *root, unsigned int flags );
	bool pfgen( lk::node_t *root, unsigned int flags );
};

}; // namespace lk

#endif
