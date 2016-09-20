// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk 
// license agreement provided at the time of installation or download, 
// or which otherwise accompanies this software in either electronic 
// or hard copy form.

#ifndef __XGENMENTALRAYPROCEDURAL_H__
#define __XGENMENTALRAYPROCEDURAL_H__

#include <vector>
#include <shader.h>
#include <memory>

#include <XGen/XgRenderAPI.h>
using namespace XGenRenderAPI;

typedef miTag miString;

namespace XGenMR
{
	class ProceduralWrapper;

	struct miMatrixClass
	{
		miMatrix m;
	};

	struct miMatrixStruct
	{
		miMatrixStruct( const miMatrix m )
		{
			mi_matrix_copy( m_matrix, m );
		}
		miMatrixStruct( const miMatrixStruct& m )
		{
			*this = m;
		}
		miMatrixStruct& operator=( const miMatrixStruct& m )
		{
			mi_matrix_copy( m_matrix, m.m_matrix );
			return *this;
		}

		/*miMatrix& operator()
		{
			return m_matrix;
		}

		const miMatrix& operator() const
		{
			return m_matrix;
		}
*/
		miMatrix m_matrix;
	};

	// Registration user data class used for pushing the data on the hair scalars
	// This stores if the data needs to be pushed, and a buffer pointer to it.
	template< class T/*user data type*/, class E/*enum for the primitive cache*/, int N/*Number of components.*/ >
	class TUserData
	{
	public:
		TUserData( PrimitiveCache* pc, PrimitiveCache::EBoolAttribute bAdd, E eArray, int context, const char* name );

		TUserData( const T* p, int context, const char* name );

		inline T get(size_t j) const;

		inline void registerUserData( size_t& io_perPrim, size_t& io_perPoint, XGenMR::UserDataFormat& fmt, size_t& k, int* interpolateComponent );
		inline void registerUserData( size_t& io_perPrim, size_t& io_perPoint, XGenMR::UserDataFormat& fmt );

		inline void copy( size_t j, float* cur, size_t& k, int context );

		inline bool isValid( int context );

	private:
		int			m_context;
		bool		m_add;
		const T*	m_p;
		std::string m_name;
	};

	typedef TUserData<int,PrimitiveCache::EIntArrayAttribute,1> TIntUserData;
	typedef TUserData<float,PrimitiveCache::EFloatArrayAttribute,1> TFloatUserData;
	typedef TUserData<vec3,PrimitiveCache::EVec3ArrayAttribute,3> TVec3UserData;

	typedef std::vector< TIntUserData >		TIntUserDataList;
	typedef std::vector< TFloatUserData >	TFloatUserDataList;
	typedef std::vector< TVec3UserData >	TVec3UserDataList;

	// Holds the 3 user data type lists + some helper functions.
	class UserDataList
	{
	public:
		// Iterate over all the user data vectors
		inline void pushUserData( size_t j, float* cur, size_t& k, int context );

		// List all the optional built in user data
		inline void listUserData( PrimitiveCache* pc, bool isSpline );

		// Register Hair Optional User Data
		inline void registerUserData( size_t& io_perPrim, size_t& io_perPoint, XGenMR::UserDataFormat& fmt, size_t& k, int* interpolateComponent );

		// Register Hair Optional User Data
		inline void registerUserData( size_t& io_perPrim, size_t& io_perPoint, XGenMR::UserDataFormat& fmt );

	private:
		TIntUserDataList vecInt;
		TFloatUserDataList vecFloat;
		TVec3UserDataList vecVec3;
	};


    
    class UserDataList; // forward declaration
    class Procedural; // forward declaration

    class BaseProcedural : public ProceduralCallbacks
    {
    public:
        BaseProcedural( const BaseProcedural* parentProc );
        virtual ~BaseProcedural();

        virtual bool get( EBoolAttribute ) const;
        virtual const char* get( EStringAttribute ) const;
        virtual float get( EFloatAttribute ) const;
        virtual const float* get( EFloatArrayAttribute ) const;
        virtual unsigned int getSize( EFloatArrayAttribute ) const;

        virtual const char* getOverride( const char* in_name ) const;
        virtual bool getArchiveBoundingBox( const char* in_filename, bbox& out_bbox ) const;
        virtual void getTransform( float in_time, mat44& out_mat ) const;

    private:
        BaseProcedural( );
        static void convertMatrix( const miMatrix in_mat, mat44& out_mat );

    protected:

        // We'll put all the attributes from user/options/camera in a param map.
        // The Parameters maps are initialized a single time in init().
        class Param
        {
        public:
            ~Param()
            {
                if( isString() )
                {
                    std::string* p = (std::string*)m_p;
                    delete p;
                }
                else if( isFloat() )
                {
                    float* p = (float*)m_p;
                    delete p;
                }
                else if( isFloatArray() )
                {
                    std::vector<float>* p = (std::vector<float>*)m_p;
                    delete p;
                }
                else if( isMatrixArray() )
                {
                    std::vector<miMatrixStruct>* p = (std::vector<miMatrixStruct>*)m_p;
                    delete p;
                }
                m_p = NULL;
            }
            Param( const std::string& in_str )
            {
                m_t = eString;
                std::string* s = new std::string;
                *s = in_str;
                m_p = (void*)s;
            }
            bool isString() { return m_t==eString; }
            std::string* asString()
            {
                return m_t==eString ? (std::string*)m_p : NULL;
            }

            Param( float in_f )
            {
                m_t = eFloat;
                float* f = new float;
                *f = in_f;
                m_p = (void*)f;
            }
            bool isFloat() { return m_t==eFloat; }
            float* asFloat()
            {
                return m_t==eFloat ? (float*)m_p : NULL;
            }

            Param( const std::vector<float>& in_f )
            {
                m_t = eFloatArray;
                std::vector<float>* f = new std::vector<float>;
                *f = in_f;
                m_p = (void*)f;
            }
            bool isFloatArray() { return m_t==eFloatArray; }
            std::vector<float>* asFloatArray()
            {
                return m_t==eFloatArray ? (std::vector<float>*)m_p : NULL;
            }

            Param( const std::vector<miMatrixStruct>& in_m )
            {
                m_t = eMatrixArray;
                std::vector<miMatrixStruct>* m = new std::vector<miMatrixStruct>;
                *m = in_m;
                m_p = (void*)m;
            }
            bool isMatrixArray() { return m_t==eMatrixArray; }
            std::vector<miMatrixStruct>* asMatrixArray()
            {
                return m_t==eMatrixArray ? (std::vector<miMatrixStruct>*)m_p : NULL;
            }
        private:
            enum ParamType
            {
                eString, eFloat, eFloatArray, eMatrixArray
            };
            ParamType m_t;
            void* m_p;
        };
        typedef std::map<std::string,Param*> ParamMap;
        ParamMap m_user, m_overrides;

        bool getString( const ParamMap& in_params, const char* in_name, const char*& out_value, bool in_user=false ) const;
        bool getFloat( const ParamMap& in_params, const char* in_name, float& out_value, bool in_user=false  ) const;

        unsigned int getArraySize( const ParamMap& in_params, const char* in_name, int in_eType, bool in_user=false  ) const;
        bool getFloatArray( const ParamMap& in_params, const char* in_name, const float*& out_value, bool in_user=false  ) const;
        bool getMatrixArray( const ParamMap& in_params, const char* in_name, const miMatrix*& out_value, bool in_user=false  ) const;
    };

    // A face renderer is created by enumerating the Faces on the PatchRenderer.
    // It will take a snapshot of the state of the PatchRenderer.
    // It will also inherit the xgen args and ProceduralCallbacks from the patch.
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    class CountHairDataProcedural : public BaseProcedural
    {
    public:

        CountHairDataProcedural( const Procedural& parentProc );
        virtual ~CountHairDataProcedural() {}
        void reset() {
            m_numPrims = 0;
            m_numPoints = 0;
        }

        // static function for initialization
        static bool initFaceRenderer(CountHairDataProcedural* dProc, 
            PatchRenderer* patch, unsigned int f);

		// XGenRenderAPI::ProceduralCallbacks
		virtual void flush(  const char* in_geom, PrimitiveCache* in_cache );
		virtual void log( const char* in_str ){}

        void render() { m_face->render();}

        inline unsigned int getNumPrims() const { return m_numPrims;}
        inline unsigned int getNumPoints() const { return m_numPoints;}

    private:
        CountHairDataProcedural();
        CountHairDataProcedural( const CountHairDataProcedural& );

        std::auto_ptr<FaceRenderer> m_face;

        // approximation settings
		miInteger m_approx_degree;
		miInteger m_approx_mode;
		miInteger m_approx_parametric_subdivisions;
		miScalar  m_approx_fine_sub_pixel_size;

        unsigned int m_numPrims;
        unsigned int m_numPoints;
    };

       // A face renderer is created by enumerating the Faces on the PatchRenderer.
    // It will take a snapshot of the state of the PatchRenderer.
    // It will also inherit the xgen args and ProceduralCallbacks from the patch.
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    class InitHairDataProcedural : public BaseProcedural
    {
    public:

        InitHairDataProcedural( const Procedural& parentProc, UserDataList* pUserData, miHair_list* pHair);
        virtual ~InitHairDataProcedural() {}

        // static function for initialization
        static bool initFaceRenderer(InitHairDataProcedural* dProc, 
            PatchRenderer* patch, unsigned int f);

		// XGenRenderAPI::ProceduralCallbacks
		virtual void flush(  const char* in_geom, PrimitiveCache* in_cache );
		virtual void log( const char* in_str ){}

        void render() { m_face->render();}

        inline bool isValid() { return m_numInterpolateComponents != -1;}

        inline miGeoIndex getNumScalarsPerPoint() const { return m_numScalarsPerPoint;}
        inline miGeoIndex getNumScalarsPerPrim() const { return m_numScalarsPerPrim;}
        inline miGeoIndex getNumInterpolateComponents() const { return m_numInterpolateComponents;} 
        inline const std::string& getUserDataStrFormat() const { return m_userDataStrFormat;}

    private:
        InitHairDataProcedural();
        InitHairDataProcedural( const InitHairDataProcedural& );

        std::auto_ptr<FaceRenderer> m_face;

        // approximation settings
		miInteger m_approx_degree;
		miInteger m_approx_mode;
		miInteger m_approx_parametric_subdivisions;
		miScalar  m_approx_fine_sub_pixel_size;

        // this user data list will be initialized
        UserDataList* m_pUserData;
        std::string m_userDataStrFormat;
        miHair_list* m_pHair;

        // data layout numbers
        miGeoIndex m_numScalarsPerPoint;
        miGeoIndex m_numScalarsPerPrim;
        miGeoIndex m_numInterpolateComponents;
    };

    class FlushHairDataProcedural : public BaseProcedural
    {
    public:

        FlushHairDataProcedural(
            const Procedural& parentProc,
            miGeoIndex* pHairIndices,
            miScalar* pHairScalars,
            UserDataList* pUserData,
            miGeoIndex numScalarsPerPoint,
            miGeoIndex numScalarsPerPrim,
            miGeoIndex numInterpolateComponents,
            miGeoIndex numScalarsTotal,
            miGeoIndex numIndicesTotal);

        virtual ~FlushHairDataProcedural() {}

        // static function for initialization
        static bool initFaceRenderer(FlushHairDataProcedural* dProc, 
            PatchRenderer* patch, unsigned int f);

		// XGenRenderAPI::ProceduralCallbacks
		virtual void flush(  const char* in_geom, PrimitiveCache* in_cache );
		virtual void log( const char* in_str ){}

        void render() { m_face->render();}

    private:
        FlushHairDataProcedural();
        FlushHairDataProcedural( const FlushHairDataProcedural& );

        std::auto_ptr<FaceRenderer> m_face;
        
        // approximation settings
		miInteger m_approx_degree;
		miInteger m_approx_mode;
		miInteger m_approx_parametric_subdivisions;
		miScalar  m_approx_fine_sub_pixel_size;
        miScalar  m_motion_blur_multiplier;
        
        // buffers to write the data to
        miGeoIndex* m_pHairIndices;
        miScalar* m_pHairScalars;
        UserDataList* m_pUserData;

        // data layout sizes
        miGeoIndex m_numScalarsPerPoint;
        miGeoIndex m_numScalarsPerPrim;
        miGeoIndex m_numInterpolateComponents;
        // Debug: compute offsets into the buffers to assert total size
        miGeoIndex m_hairScalarsOffset;
        miGeoIndex m_hairIndicesOffset;
        miGeoIndex m_numScalarsTotal;
        miGeoIndex m_numIndicesTotal;
    };

    class FlushSphereProcedural : public BaseProcedural
    {
    public:
        FlushSphereProcedural(
            const Procedural& parentProc, 
            miTag sphereTag);

        virtual ~FlushSphereProcedural() {}

        // static function for initialization
        static bool initFaceRenderer(FlushSphereProcedural* dProc, 
            PatchRenderer* patch, unsigned int f);
        miTag getResultTag() { return m_result;}

        // XGenRenderAPI::ProceduralCallbacks
        virtual void flush(  const char* in_geom, PrimitiveCache* in_cache );
        virtual void log( const char* in_str ){}

        void render() { m_face->render();}

    private:
        FlushSphereProcedural();
        FlushSphereProcedural( const FlushSphereProcedural& );

        std::auto_ptr<FaceRenderer> m_face;

        std::string m_parentName; // parent name copied from parent procedural 
        miTag m_sphere; 
        miTag m_result;
        std::vector<miTag> m_tags;
    };

	class Procedural : public BaseProcedural
	{
	public:
		friend class ProceduralWrapper;
		friend class FlushHairDataProcedural;
        friend class InitHairDataProcedural;
        friend class CountHairDataProcedural;

		struct Params
		{
			miString data;		// Procedural Arguments

			// User Attributes Arguments. We define everything on the geoshader node for now.
			// All their content is converted into ParamMap, in init();
			// In other renderers, most are defined as user data on options, camera and the procedural.
			miString user; 		// User RiAttribute on the geoshader
			miString overrides; // User overrides: length "0.0" width "0.0"
			miScalar frame;
			miString patches;	// List of patches

			// Echo params
			miBoolean	echo;					// Echo the whole geoshader content to an mi file. The echo_* parameters fills the miEchoOptions struct.
			miString	echo_filename;			// Filename where to echo.
			miBoolean	echo_ascii;             // non-binary output
			miInteger	echo_explode_objects;   // write objects to subfiles
			miBoolean	echo_verbatim_textures; // dump textures verbatim ?
			miInteger	echo_dont;              // EO_* bitmap: omit these
			miInteger	echo_dont_recurse;      // EO_* bitmap: no prereqs

			// Approx params
			miInteger approx_degree;
			miInteger approx_mode;
			miInteger approx_parametric_subdivisions;
			miScalar  approx_fine_sub_pixel_size;

			// Motion Blur params
			miBoolean motion_blur;
			miInteger motion_blur_mode;
			miInteger motion_blur_steps;
			miScalar  motion_blur_factor;
			miScalar  motion_blur_multiplier;

            // Misc. params
            miScalar max_displace;
            miScalar m_hair_object_size;        // Multiplier for the default hair object size 

            // Sphere primitive params
            miInteger m_sphere_subdiv_u;
            miInteger m_sphere_subdiv_v;
		};

		Procedural();
		virtual ~Procedural();

		// MR Entry points.
		void init(miState* state, Params* paras, miBoolean *inst_init_req );
		void exit( miState* state, Params* paras );
		miBoolean execute( miTag* result, miState* state, Params* paras );

		// Export geoshader content to disk.
		void echo( miTag* result, miState* state, Params* paras );

		// XGenRenderAPI::ProceduralCallbacks
		virtual void flush(  const char* in_geom, PrimitiveCache* in_cache );
		virtual void log( const char* in_str ){}

		// Render is called from the assembly callback function.
		bool render( miTag* result, const miState* state );

        // Helper function for transferring data from placeholder callbacks
        // to flushSplines and vice versa
        void setPlaceholderObjectTag(const miTag tag) { m_tagPlaceholderObject = tag;}
        miTag getUserDataTag() { return m_tagUserData;}
        const std::string& getParentName() const { return m_parentName;}

	private:

		// These 4 methods are protected by the global mutex.
		bool nextFace( bbox& b, unsigned int& f );
		bool initPatchRenderer( const char* in_params );
		bool initFaceRenderer( Procedural* pProc, unsigned int f );

		// init a new procedural per patch.
		bool initPatchProcedural( Procedural* pParent, const std::string& strPatch );
        
        // Helper function for patch-render callback for splines/hair.
		bool renderHairObject( miTag* result, const miState* state );
        // write out the hair object header
		miHair_list* beginHairObject( );
		void endHairObject( unsigned int numScalarsTotal, const std::string& strFormat );

        bool renderSphereAssembly();

		void flushCards( const char *geomName, PrimitiveCache* pc );
		void flushArchives( const char *geomName, PrimitiveCache* pc );
		void syncArchives( const char *geomName, PrimitiveCache* pc );

		//static void pushCustomParams( AtNode* in_node, PrimitiveCache* pc );

		const char* getUniqueName( char* buf, const char* basename );

		miTag m_node;

		typedef std::vector<Procedural*> TProcList;
		TProcList m_patches;
		PatchRenderer* m_patch;
		std::string m_patchName;
		FaceRenderer* m_face;

		std::string m_data;

		bool getString( const ParamMap& in_params, const char* in_name, const char*& out_value, bool in_user=false ) const;
		bool getFloat( const ParamMap& in_params, const char* in_name, float& out_value, bool in_user=false  ) const;

		unsigned int getArraySize( const ParamMap& in_params, const char* in_name, int in_eType, bool in_user=false  ) const;
		bool getFloatArray( const ParamMap& in_params, const char* in_name, const float*& out_value, bool in_user=false  ) const;
		bool getMatrixArray( const ParamMap& in_params, const char* in_name, const miMatrix*& out_value, bool in_user=false  ) const;

		miTag makeArchiveInstanceGroup( PrimitiveCache* pc, const std::string& instanceName, const std::string& instanceGroupName, const std::string& filename, const std::string& select, const std::string& material, miScalar frame, miInteger assembly );

		const miState* m_state;
		miTag m_result;
		miTag m_dummy;
        miTag m_tagUserData;
		std::string m_parentName;  
		std::string m_parentNameNoFace;
		std::string m_primType;
		std::vector<miTag> m_tags;
		std::vector<miTag> m_tagsHiddenGroup;
        
        miTag m_tagPlaceholderObject;
        miGeoIndex m_numHairPoints;
        miGeoIndex m_numHairPrims;
        unsigned int m_faceBegin;
        unsigned int m_faceEnd;

		std::map<std::string,std::string>* m_archives;

		bool m_bSyncArchives;
		bool m_bPerFaceAssemblies;
		bool m_bEcho;
		
		miInteger m_approx_degree;
		miInteger m_approx_mode;
		miInteger m_approx_parametric_subdivisions;
		miScalar  m_approx_fine_sub_pixel_size;

		bool m_motion_blur;
		miInteger m_motion_blur_mode;
		miInteger m_motion_blur_steps;
		miScalar  m_motion_blur_factor;
		miScalar  m_motion_blur_multiplier;
		miScalar  m_max_displace;
        miInteger m_sphere_subdiv_u;
        miInteger m_sphere_subdiv_v;
	};
};

#endif

