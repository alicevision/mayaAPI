/*
    Copyright 2005-2014 Intel Corporation.  All Rights Reserved.

    The source code contained or described herein and all documents related
    to the source code ("Material") are owned by Intel Corporation or its
    suppliers or licensors.  Title to the Material remains with Intel
    Corporation or its suppliers and licensors.  The Material is protected
    by worldwide copyright laws and treaty provisions.  No part of the
    Material may be used, copied, reproduced, modified, published, uploaded,
    posted, transmitted, distributed, or disclosed in any way without
    Intel's prior express written permission.

    No license under any patent, copyright, trade secret or other
    intellectual property right is granted to or conferred upon you by
    disclosure or delivery of the Materials, either expressly, by
    implication, inducement, estoppel or otherwise.  Any license under such
    intellectual property rights must be express and approved by Intel in
    writing.
*/

#ifndef _FGT_GRAPH_TRACE_IMPL_H
#define _FGT_GRAPH_TRACE_IMPL_H

#include "../tbb_profiling.h"

namespace tbb {
    namespace internal {

#if TBB_PREVIEW_FLOW_GRAPH_TRACE

static inline void fgt_internal_create_input_port( void *node, void *p, string_index name_index ) {
    itt_make_task_group( ITT_DOMAIN_FLOW, p, FLOW_INPUT_PORT, node, FLOW_NODE, name_index );
}

static inline void fgt_internal_create_output_port( void *node, void *p, string_index name_index ) {
    itt_make_task_group( ITT_DOMAIN_FLOW, p, FLOW_OUTPUT_PORT, node, FLOW_NODE, name_index );
}

template < typename TypesTuple, typename PortsTuple, int N >
struct fgt_internal_input_helper {
    static void register_port( void *node, PortsTuple &ports ) {
        fgt_internal_create_input_port( node, (void*)static_cast< tbb::flow::interface7::receiver< typename tbb::flow::tuple_element<N-1,TypesTuple>::type > * >(&(tbb::flow::get<N-1>(ports))),
                                        static_cast<tbb::internal::string_index>(FLOW_INPUT_PORT_0 + N - 1) );
        fgt_internal_input_helper<TypesTuple, PortsTuple, N-1>::register_port( node, ports );
    } 
};

template < typename TypesTuple, typename PortsTuple >
struct fgt_internal_input_helper<TypesTuple,PortsTuple,1> {
    static void register_port( void *node, PortsTuple &ports ) {
        fgt_internal_create_input_port( node, (void*)static_cast< tbb::flow::interface7::receiver< typename tbb::flow::tuple_element<0,TypesTuple>::type > * >(&(tbb::flow::get<0>(ports))),
                                        FLOW_INPUT_PORT_0 );
    } 
};

template < typename TypesTuple, typename PortsTuple, int N >
struct fgt_internal_output_helper {
    static void register_port( void *node, PortsTuple &ports ) {
        fgt_internal_create_output_port( node, (void*)static_cast< tbb::flow::interface7::sender< typename tbb::flow::tuple_element<N-1,TypesTuple>::type > * >(&(tbb::flow::get<N-1>(ports))),
                                         static_cast<tbb::internal::string_index>(FLOW_OUTPUT_PORT_0 + N - 1) ); 
        fgt_internal_output_helper<TypesTuple, PortsTuple, N-1>::register_port( node, ports );
    } 
};

template < typename TypesTuple, typename PortsTuple >
struct fgt_internal_output_helper<TypesTuple,PortsTuple,1> {
    static void register_port( void *node, PortsTuple &ports ) {
        fgt_internal_create_output_port( node, (void*)static_cast< tbb::flow::interface7::sender< typename tbb::flow::tuple_element<0,TypesTuple>::type > * >(&(tbb::flow::get<0>(ports))),
                                         FLOW_OUTPUT_PORT_0 ); 
    } 
};

template< typename NodeType >
void fgt_multioutput_node_desc( const NodeType *node, const char *desc ) {
    void *addr =  (void *)( static_cast< tbb::flow::interface7::receiver< typename NodeType::input_type > * >(const_cast< NodeType *>(node)) ); 
    itt_metadata_str_add( ITT_DOMAIN_FLOW, addr, FLOW_NODE, FLOW_OBJECT_NAME, desc ); 
}

template< typename NodeType >
static inline void fgt_node_desc( const NodeType *node, const char *desc ) {
    void *addr =  (void *)( static_cast< tbb::flow::interface7::sender< typename NodeType::output_type > * >(const_cast< NodeType *>(node)) ); 
    itt_metadata_str_add( ITT_DOMAIN_FLOW, addr, FLOW_NODE, FLOW_OBJECT_NAME, desc ); 
}

static inline void fgt_graph_desc( void *g, const char *desc ) {
    itt_metadata_str_add( ITT_DOMAIN_FLOW, g, FLOW_GRAPH, FLOW_OBJECT_NAME, desc ); 
}

static inline void fgt_body( void *node, void *body ) {
    itt_relation_add( ITT_DOMAIN_FLOW, body, FLOW_BODY, __itt_relation_is_child_of, node, FLOW_NODE );
}

template< typename OutputTuple, int N, typename PortsTuple >
static inline void fgt_multioutput_node( string_index t, void *g, void *input_port, PortsTuple &ports ) {
    itt_make_task_group( ITT_DOMAIN_FLOW, input_port, FLOW_NODE, g, FLOW_GRAPH, t ); 
    fgt_internal_create_input_port( input_port, input_port, FLOW_INPUT_PORT_0 ); 
    fgt_internal_output_helper<OutputTuple, PortsTuple, N>::register_port( input_port, ports ); 
}

template< typename OutputTuple, int N, typename PortsTuple >
static inline void fgt_multioutput_node_with_body( string_index t, void *g, void *input_port, PortsTuple &ports, void *body ) {
    itt_make_task_group( ITT_DOMAIN_FLOW, input_port, FLOW_NODE, g, FLOW_GRAPH, t ); 
    fgt_internal_create_input_port( input_port, input_port, FLOW_INPUT_PORT_0 ); 
    fgt_internal_output_helper<OutputTuple, PortsTuple, N>::register_port( input_port, ports ); 
    fgt_body( input_port, body );
}


template< typename InputTuple, int N, typename PortsTuple >
static inline void fgt_multiinput_node( string_index t, void *g, PortsTuple &ports, void *output_port) {
    itt_make_task_group( ITT_DOMAIN_FLOW, output_port, FLOW_NODE, g, FLOW_GRAPH, t ); 
    fgt_internal_create_output_port( output_port, output_port, FLOW_OUTPUT_PORT_0 ); 
    fgt_internal_input_helper<InputTuple, PortsTuple, N>::register_port( output_port, ports ); 
}

static inline void fgt_node( string_index t, void *g, void *output_port ) {
    itt_make_task_group( ITT_DOMAIN_FLOW, output_port, FLOW_NODE, g, FLOW_GRAPH, t ); 
    fgt_internal_create_output_port( output_port, output_port, FLOW_OUTPUT_PORT_0 ); 
}

static inline void fgt_node_with_body( string_index t, void *g, void *output_port, void *body ) {
    itt_make_task_group( ITT_DOMAIN_FLOW, output_port, FLOW_NODE, g, FLOW_GRAPH, t ); 
    fgt_internal_create_output_port( output_port, output_port, FLOW_OUTPUT_PORT_0 ); 
    fgt_body( output_port, body );
}


static inline void fgt_node( string_index t, void *g, void *input_port, void *output_port ) {
    fgt_node( t, g, output_port );
    fgt_internal_create_input_port( output_port, input_port, FLOW_INPUT_PORT_0 );
}

static inline void fgt_node_with_body( string_index t, void *g, void *input_port, void *output_port, void *body ) {
    fgt_node_with_body( t, g, output_port, body );
    fgt_internal_create_input_port( output_port, input_port, FLOW_INPUT_PORT_0 ); 
}


static inline void  fgt_node( string_index t, void *g, void *input_port, void *decrement_port, void *output_port ) {
    fgt_node( t, g, input_port, output_port );
    fgt_internal_create_input_port( output_port, decrement_port, FLOW_INPUT_PORT_1 ); 
}

static inline void fgt_make_edge( void *output_port, void *input_port ) {
    itt_relation_add( ITT_DOMAIN_FLOW, output_port, FLOW_OUTPUT_PORT, __itt_relation_is_predecessor_to, input_port, FLOW_INPUT_PORT);
}

static inline void fgt_remove_edge( void *output_port, void *input_port ) {
    itt_relation_add( ITT_DOMAIN_FLOW, output_port, FLOW_OUTPUT_PORT, __itt_relation_is_sibling_of, input_port, FLOW_INPUT_PORT);
}

static inline void fgt_graph( void *g ) {
    itt_make_task_group( ITT_DOMAIN_FLOW, g, FLOW_GRAPH, NULL, FLOW_NULL, FLOW_GRAPH ); 
}

static inline void fgt_begin_body( void *body ) {
    itt_task_begin( ITT_DOMAIN_FLOW, body, FLOW_BODY, NULL, FLOW_NULL, FLOW_NULL );
}

static inline void fgt_end_body( void * ) {
    itt_task_end( ITT_DOMAIN_FLOW );
}

#else // TBB_PREVIEW_FLOW_GRAPH_TRACE

static inline void fgt_graph( void * /*g*/ ) { }

template< typename NodeType >
static inline void fgt_multioutput_node_desc( const NodeType * /*node*/, const char * /*desc*/ ) { }

template< typename NodeType >
static inline void fgt_node_desc( const NodeType * /*node*/, const char * /*desc*/ ) { }

static inline void fgt_graph_desc( void * /*g*/, const char * /*desc*/ ) { }

static inline void fgt_body( void * /*node*/, void * /*body*/ ) { }

template< typename OutputTuple, int N, typename PortsTuple > 
static inline void fgt_multioutput_node( string_index /*t*/, void * /*g*/, void * /*input_port*/, PortsTuple & /*ports*/ ) { }

template< typename OutputTuple, int N, typename PortsTuple >
static inline void fgt_multioutput_node_with_body( string_index /*t*/, void * /*g*/, void * /*input_port*/, PortsTuple & /*ports*/, void * /*body*/ ) { }

template< typename InputTuple, int N, typename PortsTuple >
static inline void fgt_multiinput_node( string_index /*t*/, void * /*g*/, PortsTuple & /*ports*/, void * /*output_port*/ ) { }

static inline void fgt_node( string_index /*t*/, void * /*g*/, void * /*output_port*/ ) { } 
static inline void fgt_node( string_index /*t*/, void * /*g*/, void * /*input_port*/, void * /*output_port*/ ) { } 
static inline void  fgt_node( string_index /*t*/, void * /*g*/, void * /*input_port*/, void * /*decrement_port*/, void * /*output_port*/ ) { }

static inline void fgt_node_with_body( string_index /*t*/, void * /*g*/, void * /*output_port*/, void * /*body*/ ) { }
static inline void fgt_node_with_body( string_index /*t*/, void * /*g*/, void * /*input_port*/, void * /*output_port*/, void * /*body*/ ) { }

static inline void fgt_make_edge( void * /*output_port*/, void * /*input_port*/ ) { }
static inline void fgt_remove_edge( void * /*output_port*/, void * /*input_port*/ ) { }

static inline void fgt_begin_body( void * /*body*/ ) { }
static inline void fgt_end_body( void *  /*body*/) { }

#endif // TBB_PREVIEW_FLOW_GRAPH_TRACE

    } // namespace internal
} // namespace tbb

#endif
