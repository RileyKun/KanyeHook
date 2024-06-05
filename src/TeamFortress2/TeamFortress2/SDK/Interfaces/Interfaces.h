#pragma once


#include "EngineClient/EngineClient.h"
#include "EngineEffects/EngineEffects.h"
#include "BaseClientDLL/BaseClientDLL.h"
#include "ClientDLLSharedAppSystems/ClientDLLSharedAppSystems.h"
#include "ClientModeShared/ClientModeShared.h"
#include "GlobalVarsBase/GlobalVarsBase.h"
#include "ClientEntityList/ClientEntityList.h"
#include "ModelInfo/ModelInfo.h"
#include "DebugOverlay/DebugOverlay.h"
#include "EngineTrace/EngineTrace.h"
#include "GameMovement/GameMovement.h"
#include "MoveHelper/MoveHelper.h"
#include "ConVar/ConVar.h"
#include "Prediction/Prediction.h"
#include "Panel/Panel.h"
#include "Surface/Surface.h"
#include "EngineVGui/EngineVGui.h"
#include "NetChannel/NetChannel.h"
#include "Steam/Steamworks.h"
#include "Steam/SteamTypes.h"
#include "GameEvent/GameEvent.h"
#include "Material/Material.h"
#include "ModelRender/ModelRender.h"
#include "ViewRender/ViewRender.h"
#include "RenderView/RenderView.h"
#include "Input/Input.h"
#include "KeyValuesSystem/KeyValuesSystem.h"
#include "UniformRandomStream/UniformRandomStream.h"
#include "InputSystem/InputSystem.h"
#include "EffectsClient/EffectsClient.h"
#include "AchievementMgr/AchievementMgr.h"
#include "ViewRenderBeams/ViewRenderBeams.h"
#include "EngineSound/EngineSound.h"

#include "Steam/ISteamNetworkingUtils.h"
#include "CTFGCClientSystem/CTFGCClientSystem.h"
#include "CTFPartyClient/CTFPartyClient.h"
#include "CTFInventoryManager/CTFInventoryManager.h"
#include "../DirectX/DirectX.h"

struct vcollide_t;

enum {
	COLLIDE_POLY = 0,
	COLLIDE_MOPP = 1,
	COLLIDE_BALL = 2,
	COLLIDE_VIRTUAL = 3,
};

class i_phys_collide {
public:
	virtual ~i_phys_collide() { }
	virtual void* create_surface_manager(short&) const = 0;
	virtual void get_all_ledges(void* ledges) const = 0;
	virtual unsigned int get_serialization_size() const = 0;
	virtual unsigned int serialize_to_buffer(char* dest, bool swap = false) const = 0;
	virtual int get_vcollide_index() const = 0;
	virtual Vector get_mass_center() const = 0;
	virtual void set_mass_center(const Vector& mass_center) = 0;
	virtual Vector get_orthographic_areas() const = 0;
	virtual void set_orthographic_areas(const Vector& areas) = 0;
	virtual float get_sphere_radius() const = 0;
	virtual void output_debug_info() const = 0;
};

#define LEAFMAP_HAS_CUBEMAP					0x0001
#define LEAFMAP_HAS_SINGLE_VERTEX_SPAN		0x0002
#define LEAFMAP_HAS_MULTIPLE_VERTEX_SPANS	0x0004

struct leafmap_t {
	void* m_leaf;
	unsigned short m_vert_count;
	byte m_flags;
	byte m_span_count;
	unsigned short m_start_vert[8];

	void set_has_cubemap() {
		this->m_flags = LEAFMAP_HAS_CUBEMAP;
	}

	void set_single_vertex_span(int start_vert_index, int vert_count_in) {
		this->m_flags = 0;
		this->m_flags |= LEAFMAP_HAS_SINGLE_VERTEX_SPAN;
		this->m_start_vert[0] = start_vert_index;
		this->m_vert_count = vert_count_in;
	}

	int max_spans() {
		return sizeof(m_start_vert) - sizeof(m_start_vert[0]);
	}
	const byte* get_spans() const {
		return reinterpret_cast<const byte*>(&m_start_vert[1]);
	}
	byte* get_spans() {
		return reinterpret_cast<byte*>(&m_start_vert[1]);
	}

	void set_rle_spans(int start_vert_index, int span_count_in, byte* spans) {
		m_flags = 0;

		if (span_count_in > max_spans())
			return;

		if (span_count_in == 1) {
			set_single_vertex_span(start_vert_index, spans[0]);
			return;
		}

		m_flags |= LEAFMAP_HAS_MULTIPLE_VERTEX_SPANS;
		m_start_vert[0] = start_vert_index;
		m_vert_count = 0;
		m_span_count = span_count_in;

		byte* span_out = get_spans();

		for (int i = 0; i < span_count_in; i++) {
			span_out[i] = spans[i];

			if (!(i & 1)) {
				m_vert_count += spans[i];
			}
		}
	}

	inline bool has_spans() const { return (m_flags & (LEAFMAP_HAS_SINGLE_VERTEX_SPAN | LEAFMAP_HAS_MULTIPLE_VERTEX_SPANS)) ? true : false; }
	inline bool has_cubemap() const { return (m_flags & LEAFMAP_HAS_CUBEMAP) ? true : false; }
	inline bool has_single_vertex_span() const { return (m_flags & LEAFMAP_HAS_SINGLE_VERTEX_SPAN) ? true : false; }
	inline bool has_rle_spans() const { return (m_flags & LEAFMAP_HAS_MULTIPLE_VERTEX_SPANS) ? true : false; }
};


struct collidemap_t {
	int m_leaf_count;
	leafmap_t m_leafmap[1];
};

class c_phys_collide : public i_phys_collide {
public:
	static c_phys_collide* unserialize_from_buffer(const char* buffer, unsigned int size, int index, bool swap = false);
	virtual const void* get_compact_surface() const { return NULL; }
	virtual Vector get_orthographic_areas() const { return Vector(1.0f, 1.0f, 1.0f); }
	virtual float get_sphere_radius() const { return 0; }
	virtual void compute_orthographic_areas(float epsilon) {}
	virtual void set_orthographic_areas(const Vector& areas) {}
	virtual const collidemap_t* get_collide_map() const { return NULL; }
};

#define METERS_PER_INCH					( 0.0254f )
#define CUBIC_METERS_PER_CUBIC_INCH		( METERS_PER_INCH*METERS_PER_INCH*METERS_PER_INCH )
#define POUNDS_PER_KG	( 2.2f )
#define KG_PER_POUND	( 1.0f / POUNDS_PER_KG )
#define LBS2KG( x )		( ( x ) * KG_PER_POUND )
#define KG2LBS( x )		( ( x ) * POUNDS_PER_KG )

const float vphysics_min_mass = 0.1f;
const float vphysics_max_mass = 5e4f;

class i_physics_object {
public:
	virtual ~i_physics_object(void) { }
	virtual bool is_static() const = 0;
	virtual bool is_asleep() const = 0;
	virtual bool is_trigger() const = 0;
	virtual bool is_fluid() const = 0;
	virtual bool is_hinged() const = 0;
	virtual bool is_collision_enabled() const = 0;
	virtual bool is_gravity_enabled() const = 0;
	virtual bool is_drag_enabled() const = 0;
	virtual bool is_motion_enabled() const = 0;
	virtual bool is_moveable() const = 0;
	virtual bool is_attached_to_constraint(bool external_only) const = 0;
	virtual void enable_collisions(bool enable) = 0;
	virtual void enable_gravity(bool enable) = 0;
	virtual void enable_drag(bool enable) = 0;
	virtual void enable_motion(bool enable) = 0;
	virtual void set_game_data(void* game_data) = 0;
	virtual void* get_game_data(void) const = 0;
	virtual void set_game_flags(unsigned short user_flags) = 0;
	virtual unsigned short get_game_flags(void) const = 0;
	virtual void set_game_index(unsigned short game_index) = 0;
	virtual unsigned short get_game_index(void) const = 0;
	virtual void set_callback_flags(unsigned short callbackflags) = 0;
	virtual unsigned short get_callback_flags(void) const = 0;
	virtual void wake(void) = 0;
	virtual void sleep(void) = 0;
	virtual void recheck_collision_filter() = 0;
	virtual void recheck_contact_points() = 0;
	virtual void set_mass(float mass) = 0;
	virtual float get_mass(void) const = 0;
	virtual float get_inv_mass(void) const = 0;
	virtual Vector get_inertia(void) const = 0;
	virtual Vector get_inv_inertia(void) const = 0;
	virtual void set_inertia(const Vector& inertia) = 0;
	virtual void set_damping(const float* speed, const float* rot) = 0;
	virtual void get_damping(float* speed, float* rot) const = 0;
	virtual void set_drag_coefficient(float* pDrag, float* angular_drag) = 0;
	virtual void set_buoyancy_ratio(float ratio) = 0;
	virtual int get_material_index() const = 0;
	virtual void set_material_index(int material_index) = 0;
	virtual unsigned int get_contents() const = 0;
	virtual void set_contents(unsigned int contents) = 0;
	virtual float get_sphere_radius() const = 0;
	virtual float get_energy() const = 0;
	virtual Vector get_mass_center_local_space() const = 0;
	virtual void set_position(const Vector& world_position, const Vector& angles, bool is_teleport) = 0;
	virtual void set_position_matrix(const matrix3x4& matrix, bool is_teleport) = 0;
	virtual void get_position(Vector* world_position, Vector* angles) const = 0;
	virtual void get_position_matrix(matrix3x4* position_matrix) const = 0;
	virtual void set_velocity(const Vector* velocity, const Vector* angular_velocity) = 0;
	virtual void set_velocity_instantaneous(const Vector* velocity, const Vector* angular_velocity) = 0;
	virtual void get_velocity(Vector* velocity, Vector* angular_velocity) const = 0;
	virtual void add_velocity(const Vector* velocity, const Vector* angular_velocity) = 0;
	virtual void get_velocity_at_point(const Vector& world_position, Vector* velocity) const = 0;
	virtual void get_implicit_velocity(Vector* velocity, Vector* angular_velocity) const = 0;
	virtual void local_to_world(Vector* world_position, const Vector& local_position) const = 0;
	virtual void world_to_local(Vector* local_position, const Vector& world_position) const = 0;
	virtual void local_to_world_Vector(Vector* world_Vector, const Vector& local_Vector) const = 0;
	virtual void world_to_local_Vector(Vector* local_Vector, const Vector& world_Vector) const = 0;
	virtual void apply_force_center(const Vector& force_Vector) = 0;
	virtual void apply_force_offset(const Vector& force_Vector, const Vector& world_position) = 0;
	virtual void apply_torque_center(const Vector& torque) = 0;
	virtual void calculate_force_offset(const Vector& force_Vector, const Vector& world_position, Vector* center_force, Vector* center_torque) const = 0;
	virtual void calculate_velocity_offset(const Vector& force_Vector, const Vector& world_position, Vector* center_velocity, Vector* center_angular_velocity) const = 0;
	virtual float calculate_linear_drag(const Vector& unit_direction) const = 0;
	virtual float calculate_angular_drag(const Vector& object_space_rotation_axis) const = 0;
	virtual bool get_contact_point(Vector* contactPoint, i_physics_object** contact_object) const = 0;
	virtual void set_shadow(float max_speed, float max_angular_speed, bool allow_physics_movement, bool allow_physics_rotation) = 0;
	virtual void update_shadow(const Vector& target_position, const Vector& target_angles, bool temp_disable_gravity, float time_offset) = 0;
	virtual int get_shadow_position(Vector* position, Vector* angles) const = 0;
	virtual void* get_shadow_controller(void) const = 0;
	virtual void remove_shadow_controller() = 0;
	virtual float compute_shadow_control(void* params, float seconds_to_arrival, float dt) = 0;
	virtual const c_phys_collide* get_collide(void) const = 0;
	virtual const char* get_name() const = 0;
	virtual void become_trigger() = 0;
	virtual void remove_trigger() = 0;
	virtual void become_hinged(int local_axis) = 0;
	virtual void remove_hinged() = 0;
	virtual void* create_friction_snapshot() = 0;
	virtual void destroy_friction_snapshot(void* snapshot) = 0;
	virtual void output_debug_info() const = 0;
};

class i_physics_environment;
class i_physics_surface_props;
class i_physics_constraint;
class i_physics_constraint_group;
class i_physics_fluid_controller;
class i_physics_spring;
class i_physics_vehicle_controller;
class i_convex_info;
class i_physics_object_pair_hash;
class i_physics_collision_set;
class i_physics_player_controller;
class i_physics_friction_snapshot;
struct constraint_ragdollparams_t;
struct constraint_hingeparams_t;
struct constraint_fixedparams_t;
struct constraint_ballsocketparams_t;
struct constraint_slidingparams_t;
struct constraint_pulleyparams_t;
struct constraint_lengthparams_t;
struct constraint_groupparams_t;
struct vehicleparams_t;
struct fluidparams_t;
struct springparams_t;

struct objectparams_t {
	Vector* m_mass_center_override;
	float		m_mass;
	float		m_inertia;
	float		m_damping;
	float		m_rot_damping;
	float		m_rot_inertia_limit;
	const char* m_name;
	void* m_game_data;
	float		m_volume;
	float		m_drag_coefficient;
	bool		m_enable_collisions;
};

struct debugcollide_t;
class c_game_trace;
typedef c_game_trace trace_t;
struct physics_stats_t;
struct physics_performanceparams_t;
struct virtualmeshparams_t;
struct physsaveparams_t;
struct physrestoreparams_t;
struct physprerestoreparams_t;

enum phys_interface_id_t {
	PIID_UNKNOWN,
	PIID_IPHYSICSOBJECT,
	PIID_I_PHYSICS_FLUID_CONTROLLER,
	PIID_I_PHYSICS_SPRING,
	PIID_I_PHYSICS_CONSTRAINT_GROUP,
	PIID_I_PHYSICS_CONSTRAINT,
	PIID_I_PHYSICS_SHADOW_CONTROLLER,
	PIID_I_PHYSICS_PLAYER_CONTROLLER,
	PIID_I_PHYSICS_MOTION_CONTROLLER,
	PIID_I_PHYSICS_VEHICLE_CONTROLLER,
	PIID_I_PHYSICS_GAME_TRACE,
	PIID_NUM_TYPES
};

class i_save;
class i_restore;

class i_vphysics_debug_overlay {
public:
	virtual void add_entity_text_overlay(int ent_index, int line_offset, float duration, int r, int g, int b, int a, const char* format, ...) = 0;
	virtual void add_box_overlay(const Vector& origin, const Vector& mins, const Vector& max, Vector const& orientation, int r, int g, int b, int a, float duration) = 0;
	virtual void add_triangle_overlay(const Vector& p1, const Vector& p2, const Vector& p3, int r, int g, int b, int a, bool no_depth_test, float duration) = 0;
	virtual void add_line_overlay(const Vector& origin, const Vector& dest, int r, int g, int b, bool no_depth_test, float duration) = 0;
	virtual void add_text_overlay(const Vector& origin, float duration, const char* format, ...) = 0;
	virtual void add_text_overlay(const Vector& origin, int line_offset, float duration, const char* format, ...) = 0;
	virtual void add_screen_text_overlay(float x_pos, float y_pos, float duration, int r, int g, int b, int a, const char* text) = 0;
	virtual void add_swept_box_overlay(const Vector& start, const Vector& end, const Vector& mins, const Vector& max, const Vector& angles, int r, int g, int b, int a, float duration) = 0;
	virtual void add_text_overlay_rgb(const Vector& origin, int line_offset, float duration, float r, float g, float b, float alpha, const char* format, ...) = 0;
};


typedef void* (*create_interface_fn)(const char* name, int* return_code);
typedef void* (*instantiate_interface_fn)();

enum init_return_val_t {
	INIT_FAILED = 0,
	INIT_OK,
	INIT_LAST_VAL,
};

class i_app_system {
public:
	virtual bool connect(create_interface_fn factory) = 0;
	virtual void disconnect() = 0;
	virtual void* query_interface(const char* interface_name) = 0;
	virtual init_return_val_t init() = 0;
	virtual void shutdown() = 0;
};

class i_physics : public i_app_system {
public:
	virtual	i_physics_environment* create_environment(void) = 0;
	virtual void destroy_environment(i_physics_environment*) = 0;
	virtual i_physics_environment* get_active_environment_by_index(int index) = 0;
	virtual i_physics_object_pair_hash* create_object_pair_hash() = 0;
	virtual void destroy_object_pair_hash(i_physics_object_pair_hash* hash) = 0;
	virtual i_physics_collision_set* find_or_create_collision_set(unsigned int id, int max_element_count) = 0;
	virtual i_physics_collision_set* find_collision_set(unsigned int id) = 0;
	virtual void destroy_all_collision_sets() = 0;
};

class c_phys_convex;
class c_phys_polysoup;
class i_collision_query;
class i_vphysics_key_parser;
struct convertconvexparams_t;
class c_packed_physics_description;
class c_polyhedron;

struct truncatedcone_t {
	Vector m_origin;
	Vector m_normal;
	float m_h;
	float m_theta;
};

class i_physics_collision {
public:
	virtual ~i_physics_collision(void) { }
	virtual c_phys_convex* convex_from_verts(Vector** verts, int vert_count) = 0;
	virtual c_phys_convex* convex_from_planes(float* planes, int plane_count, float merge_distance) = 0;
	virtual float convex_volume(c_phys_convex* convex) = 0;
	virtual float convex_surface_area(c_phys_convex* convex) = 0;
	virtual void set_convex_game_data(c_phys_convex* convex, unsigned int game_data) = 0;
	virtual void convex_free(c_phys_convex* convex) = 0;
	virtual c_phys_convex* bbox_to_convex(const Vector& mins, const Vector& maxs) = 0;
	virtual c_phys_convex* convex_from_convex_polyhedron(const c_polyhedron& convex_polyhedron) = 0;
	virtual void convexes_from_convex_polygon(const Vector& poly_normal, const Vector* points, int point_count, c_phys_convex** output) = 0;
	virtual c_phys_polysoup* polysoup_create(void) = 0;
	virtual void polysoup_destroy(c_phys_polysoup* soup) = 0;
	virtual void polysoup_add_triangle(c_phys_polysoup* soup, const Vector& a, const Vector& b, const Vector& c, int material_index7bits) = 0;
	virtual c_phys_collide* convert_polysoup_to_collide(c_phys_polysoup* soup, bool use_mopp) = 0;
	virtual c_phys_collide* convert_convex_to_collide(c_phys_convex** convex, int convex_count) = 0;
	virtual c_phys_collide* convert_convex_to_collide_params(c_phys_convex** convex, int convex_count, const convertconvexparams_t& convert_params) = 0;
	virtual void destroy_collide(c_phys_collide* collide) = 0;
	virtual int collide_size(c_phys_collide* collide) = 0;
	virtual int collide_write(char* pDest, c_phys_collide* collide, bool swap = false) = 0;
	virtual c_phys_collide* unserialize_collide(char* buffer, int size, int index) = 0;
	virtual float collide_volume(c_phys_collide* collide) = 0;
	virtual float collide_surface_area(c_phys_collide* collide) = 0;
	virtual Vector collide_get_extent(const c_phys_collide* collide, const Vector& collide_origin, const Vector& collide_angles, const Vector& direction) = 0;
	virtual void collide_get_aabb(Vector* mins, Vector* maxs, const c_phys_collide* collide, const Vector& collide_origin, const Vector& collide_angles) = 0;
	virtual void collide_get_mass_center(c_phys_collide* collide, Vector* out_mass_center) = 0;
	virtual void collide_set_mass_center(c_phys_collide* collide, const Vector& mass_center) = 0;
	virtual Vector collide_get_orthographic_areas(const c_phys_collide* collide) = 0;
	virtual void collide_set_orthographic_areas(c_phys_collide* collide, const Vector& areas) = 0;
	virtual int collide_index(const c_phys_collide* collide) = 0;
	virtual c_phys_collide* bbox_to_collide(const Vector& mins, const Vector& maxs) = 0;
	virtual int get_convexes_used_in_collideable(const c_phys_collide* collideable, c_phys_convex** output_array, int output_array_limit) = 0;
	virtual void trace_box(const Vector& start, const Vector& end, const Vector& mins, const Vector& maxs, const c_phys_collide* collide, const Vector& collide_origin, const Vector& collide_angles, trace_t* ptr) = 0;
	virtual void trace_box(const Ray_t& ray, const c_phys_collide* collide, const Vector& collide_origin, const Vector& collide_angles, trace_t* ptr) = 0;
	virtual void trace_box(const Ray_t& ray, unsigned int contents_mask, i_convex_info* convex_info, const c_phys_collide* collide, const Vector& collide_origin, const Vector& collide_angles, trace_t* ptr) = 0;
	virtual void trace_collide(const Vector& start, const Vector& end, const c_phys_collide* pSweepCollide, const Vector& sweepAngles, const c_phys_collide* pCollide, const Vector& collideOrigin, const Vector& collideAngles, trace_t* ptr) = 0;
	virtual bool is_box_intersecting_cone(const Vector& box_abs_mins, const Vector& box_abs_maxs, const truncatedcone_t& cone) = 0;
	virtual void vcollide_load(vcollide_t* output, int solid_count, const char* buffer, int size, bool swap = false) = 0;
	virtual void vcollide_unload(vcollide_t* vcollide) = 0;
	virtual i_vphysics_key_parser* vphysics_key_parser_create(const char* key_data) = 0;
	virtual void vphysics_key_parser_destroy(i_vphysics_key_parser* parser) = 0;
	virtual int create_debug_mesh(c_phys_collide const* collision_model, Vector** out_verts) = 0;
	virtual void destroy_debug_mesh(int vert_count, Vector* out_verts) = 0;
	virtual i_collision_query* create_query_model(c_phys_collide* collide) = 0;
	virtual void destroy_query_model(i_collision_query* query) = 0;
	virtual i_physics_collision* thread_context_create(void) = 0;
	virtual void thread_context_destroy(i_physics_collision* thread_contex) = 0;
	virtual c_phys_collide* create_virtual_mesh(const virtualmeshparams_t& params) = 0;
	virtual bool supports_virtual_mesh() = 0;
	virtual bool get_bbox_cache_size(int* cached_size, int* cached_count) = 0;
	virtual c_polyhedron* polyhedron_from_convex(c_phys_convex* const convex, bool use_temp_polyhedron) = 0;
	virtual void output_debug_info(const c_phys_collide* collide) = 0;
	virtual unsigned int read_stat(int stat_id) = 0;
};

class i_collision_query {
public:
	virtual ~i_collision_query() { }
	virtual int convex_count(void) = 0;
	virtual int triangle_count(int convex_index) = 0;
	virtual unsigned int get_game_data(int convex_index) = 0;
	virtual void get_triangle_verts(int convex_index, int triangle_index, Vector* verts) = 0;
	virtual void set_triangle_verts(int convex_index, int triangle_index, const Vector* verts) = 0;
	virtual int get_triangle_material_index(int convex_index, int triangle_index) = 0;
	virtual void set_triangle_material_index(int convex_index, int triangle_index, int index7bits) = 0;
};

class i_physics_game_trace {
public:
	virtual void vehicle_trace_ray(const Ray_t& ray, void* vehicle, trace_t* trace) = 0;
	virtual	void vehicle_trace_ray_with_water(const Ray_t& ray, void* vehicle, trace_t* trace) = 0;
	virtual bool vehicle_point_in_water(const Vector& point) = 0;
};

class i_convex_info {
public:
	virtual unsigned int get_contents(int convex_game_data) = 0;
};

class c_physics_event_handler;
class i_physics_collision_data {
public:
	virtual void get_surface_normal(Vector& out) = 0;
	virtual void get_contact_point(Vector& out) = 0;
	virtual void get_contact_speed(Vector& out) = 0;
};

struct vcollisionevent_t {
	i_physics_object* m_objects[2];
	int m_surface_props[2];
	bool m_is_collision;
	bool m_is_shadow_collision;
	float m_delta_collision_time;
	float m_collision_speed;
	i_physics_collision_data* m_internal_data;
};

class i_physics_collision_event {
public:
	virtual void pre_collision(vcollisionevent_t* event_) = 0;
	virtual void post_collision(vcollisionevent_t* event_) = 0;
	virtual void friction(i_physics_object* pObject, float energy, int surface_props, int surface_props_hit, i_physics_collision_data* data) = 0;
	virtual void start_touch(i_physics_object* object, i_physics_object* object_, i_physics_collision_data* touch_data) = 0;
	virtual void end_touch(i_physics_object* object, i_physics_object* object_, i_physics_collision_data* touch_data) = 0;
	virtual void fluid_start_touch(i_physics_object* object, i_physics_fluid_controller* fluid) = 0;
	virtual void fluid_end_touch(i_physics_object* object, i_physics_fluid_controller* fluid) = 0;
	virtual void post_simulation_frame() = 0;
	virtual void object_enter_trigger(i_physics_object* trigger, i_physics_object* object) { }
	virtual void object_leave_trigger(i_physics_object* trigger, i_physics_object* object) { }
};

class i_physics_object_event {
public:
	virtual void object_wake(i_physics_object* object) = 0;
	virtual void object_sleep(i_physics_object* object) = 0;
};

class i_physics_constraint_event {
public:
	virtual void constraint_broken(i_physics_constraint*) = 0;
};

struct hlshadowcontrol_params_t {
	Vector m_target_position;
	Vector m_target_rotation;
	float m_max_angular;
	float m_max_damp_angular;
	float m_max_speed;
	float m_max_damp_speed;
	float m_damp_factor;
	float m_teleport_distance;
};

class i_physics_shadow_controller {
public:
	virtual ~i_physics_shadow_controller(void) { }

	virtual void update(const Vector& position, const Vector& angles, float time_offset) = 0;
	virtual void max_speed(float max_speed, float maxangular_speed) = 0;
	virtual void step_up(float height) = 0;
	virtual void set_teleport_distance(float teleport_distance) = 0;
	virtual bool allows_translation() = 0;
	virtual bool allows_rotation() = 0;
	virtual void set_physically_controlled(bool is_physically_controlled) = 0;
	virtual bool is_physically_controlled() = 0;
	virtual void get_last_impulse(Vector* out) = 0;
	virtual void use_shadow_material(bool use_shadow_material) = 0;
	virtual void object_material_changed(int material_index) = 0;
	virtual float get_target_position(Vector* position_out, Vector* angles_out) = 0;
	virtual float get_teleport_distance(void) = 0;
	virtual void get_max_speed(float* max_speed_out, float* max_angular_speed_out) = 0;
};

class c_physics_sim_object;
class i_physics_motion_controller;

class i_motion_event {
public:
	enum simresult_e { IM_NOTHING = 0, SIM_LOCAL_ACCELERATION, SIM_LOCAL_FORCE, SIM_GLOBAL_ACCELERATION, SIM_GLOBAL_FORCE };
	virtual simresult_e	simulate(i_physics_motion_controller* controller, i_physics_object* object, float delta_time, Vector& linear, Vector& angular) = 0;
};

class i_physics_motion_controller {
public:
	virtual ~i_physics_motion_controller(void) { }
	virtual void set_event_handler(i_motion_event* handler) = 0;
	virtual void attach_object(i_physics_object* object, bool check_if_already_attached) = 0;
	virtual void detach_object(i_physics_object* object) = 0;
	virtual int count_objects(void) = 0;
	virtual void get_objects(i_physics_object** object_list) = 0;
	virtual void clear_objects(void) = 0;
	virtual void wake_objects(void) = 0;

	enum priority_t {
		LOW_PRIORITY = 0,
		MEDIUM_PRIORITY = 1,
		HIGH_PRIORITY = 2
	};

	virtual void set_priority(priority_t priority) = 0;
};

class i_physics_collision_solver {
public:
	virtual int should_collide(i_physics_object* object, i_physics_object* object_, void* game_data, void* game_data_) = 0;
	virtual int should_solve_penetration(i_physics_object* object, i_physics_object* object_, void* game_data, void* game_data_, float dt) = 0;
	virtual bool should_freeze_object(i_physics_object* object) = 0;
	virtual int additional_collision_checks_this_tick(int current_checks_done) = 0;
	virtual bool should_freeze_contacts(i_physics_object** object_list, int object_count) = 0;
};

enum physics_trace_type_t {
	VPHYSICS_TRACE_EVERYTHING = 0,
	VPHYSICS_TRACE_STATIC_ONLY,
	VPHYSICS_TRACE_MOVING_ONLY,
	VPHYSICS_TRACE_TRIGGERS_ONLY,
	VPHYSICS_TRACE_STATIC_AND_MOVING,
};

class i_physics_trace_filter {
public:
	virtual bool should_hit_object(i_physics_object* object, int contents_mask) = 0;
	virtual physics_trace_type_t get_trace_type() const = 0;
};

class i_physics_environment {
public:
	virtual ~i_physics_environment(void) { }
	virtual void set_debug_overlay(create_interface_fn debug_overlay_factory) = 0;
	virtual i_vphysics_debug_overlay* get_debug_overlay(void) = 0;
	virtual void set_gravity(const Vector& gravity_Vector) = 0;
	virtual void get_gravity(Vector* gravity_Vector) const = 0;
	virtual void set_air_density(float density) = 0;
	virtual float get_air_density(void) const = 0;
	virtual i_physics_object* create_poly_object(const c_phys_collide* collision_model, int material_index, const Vector& position, const Vector& angles, objectparams_t* params) = 0;
	virtual i_physics_object* create_poly_object_static(const c_phys_collide* collision_model, int material_index, const Vector& position, const Vector& angles, objectparams_t* params) = 0;
	virtual i_physics_object* create_sphere_object(float radius, int material_index, const Vector& position, const Vector& angles, objectparams_t* params, bool is_static) = 0;
	virtual void destroy_object(i_physics_object*) = 0;
	virtual i_physics_fluid_controller* create_fluid_controller(i_physics_object* fluid_object, fluidparams_t* params) = 0;
	virtual void destroy_fluid_controller(i_physics_fluid_controller*) = 0;
	virtual i_physics_spring* create_spring(i_physics_object* object_start, i_physics_object* object_end, springparams_t* params) = 0;
	virtual void destroy_spring(i_physics_spring*) = 0;
	virtual i_physics_constraint* create_ragdoll_constraint(i_physics_object* reference_object, i_physics_object* attached_object, i_physics_constraint_group* group, const constraint_ragdollparams_t& ragdoll) = 0;
	virtual i_physics_constraint* create_hinge_constraint(i_physics_object* reference_object, i_physics_object* attached_object, i_physics_constraint_group* group, const constraint_hingeparams_t& hinge) = 0;
	virtual i_physics_constraint* create_fixed_constraint(i_physics_object* reference_object, i_physics_object* attached_object, i_physics_constraint_group* group, const constraint_fixedparams_t& fixed) = 0;
	virtual i_physics_constraint* create_sliding_constraint(i_physics_object* reference_object, i_physics_object* attached_object, i_physics_constraint_group* group, const constraint_slidingparams_t& sliding) = 0;
	virtual i_physics_constraint* create_ballsocket_constraint(i_physics_object* reference_object, i_physics_object* attached_object, i_physics_constraint_group* group, const constraint_ballsocketparams_t& ballsocket) = 0;
	virtual i_physics_constraint* create_pulley_constraint(i_physics_object* reference_object, i_physics_object* attached_object, i_physics_constraint_group* group, const constraint_pulleyparams_t& pulley) = 0;
	virtual i_physics_constraint* create_length_constraint(i_physics_object* reference_object, i_physics_object* attached_object, i_physics_constraint_group* group, const constraint_lengthparams_t& length) = 0;
	virtual void destroy_constraint(i_physics_constraint*) = 0;
	virtual i_physics_constraint_group* create_constraint_group(const constraint_groupparams_t& group_params) = 0;
	virtual void destroy_constraint_group(i_physics_constraint_group* group) = 0;
	virtual i_physics_shadow_controller* create_shadow_controller(i_physics_object* object, bool allow_translation, bool allow_rotation) = 0;
	virtual void destroy_shadow_controller(i_physics_shadow_controller*) = 0;
	virtual i_physics_player_controller* create_player_controller(i_physics_object* object) = 0;
	virtual void destroy_player_controller(i_physics_player_controller*) = 0;
	virtual i_physics_motion_controller* create_motion_controller(i_motion_event* handler) = 0;
	virtual void destroy_motion_controller(i_physics_motion_controller* controller) = 0;
	virtual i_physics_vehicle_controller* create_vehicle_controller(i_physics_object* vehicle_body_object, const vehicleparams_t& params, unsigned int nVehicleType, i_physics_game_trace* game_trace) = 0;
	virtual void destroy_vehicle_controller(i_physics_vehicle_controller*) = 0;
	virtual void set_collision_solver(i_physics_collision_solver* solver) = 0;
	virtual void simulate(float delta_time) = 0;
	virtual bool is_in_simulation() const = 0;
	virtual float get_simulation_timestep() const = 0;
	virtual void set_simulation_timestep(float timestep) = 0;
	virtual float get_simulation_time() const = 0;
	virtual void reset_simulation_clock() = 0;
	virtual float get_next_frame_time(void) const = 0;
	virtual void set_collision_event_handler(i_physics_collision_event* collision_events) = 0;
	virtual void set_object_event_handler(i_physics_object_event* object_events) = 0;
	virtual	void set_constraint_event_handler(i_physics_constraint_event* constraint_event) = 0;
	virtual void set_quick_delete(bool quick) = 0;
	virtual int get_active_object_count() const = 0;
	virtual void get_active_objects(i_physics_object** output_object_list) const = 0;
	virtual const i_physics_object** get_object_list(int* output_object_count) const = 0;
	virtual bool transfer_object(i_physics_object* object, i_physics_environment* destination_environment) = 0;
	virtual void cleanup_delete_list(void) = 0;
	virtual void enable_delete_queue(bool enable) = 0;
	virtual bool save(const physsaveparams_t& params) = 0;
	virtual void pre_restore(const physprerestoreparams_t& params) = 0;
	virtual bool restore(const physrestoreparams_t& params) = 0;
	virtual void post_restore() = 0;
	virtual bool is_collision_model_used(c_phys_collide* collide) const = 0;
	virtual void trace_ray(const Ray_t& ray, unsigned int mask, i_physics_trace_filter* trace_filter, trace_t* trace) = 0;
	virtual void sweep_collideable(const c_phys_collide* collide, const Vector& abs_start, const Vector& abs_end, const Vector& angles, unsigned int mask, i_physics_trace_filter* trace_filter, trace_t* trace) = 0;
	virtual void get_performance_settings(physics_performanceparams_t* output) const = 0;
	virtual void set_performance_settings(const physics_performanceparams_t* settings) = 0;
	virtual void read_stats(physics_stats_t* output) = 0;
	virtual void clear_stats() = 0;
	virtual unsigned int get_object_serialize_size(i_physics_object* object) const = 0;
	virtual void serialize_object_to_buffer(i_physics_object* object, unsigned char* buffer, unsigned int buffer_size) = 0;
	virtual i_physics_object* unserialize_object_from_buffer(void* game_data, unsigned char* buffer, unsigned int buffer_size, bool enable_collisions) = 0;
	virtual void enable_constraint_notify(bool enable) = 0;
	virtual void debug_check_contacts(void) = 0;
};

const objectparams_t g_phys_default_object_params = {
	NULL,
	1.0,
	1.0,
	0.1f,
	0.1f,
	0.05f,
	"DEFAULT",
	NULL,
	0.f,
	1.0f,
	true,
};

class c_physics_object : public i_physics_object {
public:
	void* m_game_data;
	void* m_object;
	const c_phys_collide* m_collide;
	i_physics_shadow_controller* m_shadow;
	Vector m_drag_basis;
	Vector m_angular_drag_basis;
	bool m_shadow_temp_gravity_disable : 5;
	bool m_has_touched_dynamic : 1;
	bool m_asleep_since_creation : 1;
	bool m_force_silent_delete : 1;
	unsigned char m_sleep_state : 2;
	unsigned char m_hinged_axis : 3;
	unsigned char m_collide_type : 3;
	unsigned short m_game_index;
	unsigned short m_material_index;
	unsigned short m_active_index;
	unsigned short m_callbacks;
	unsigned short m_game_flags;
	unsigned int m_contents_mask;
	float m_volume;
	float m_buoyancy_ratio;
	float m_drag_coefficient;
	float m_angular_drag_coefficient;
};

inline const float k_max_velocity = 2000.0f;
inline const float k_max_angular_velocity = 360.0f * 10.0f;

const float default_min_fricion_mass = 10.0f;
const float default_max_fricion_mass = 2500.0f;

struct physics_performanceparams_t {
	int m_max_collisions_per_object_per_timestep;
	int m_max_collision_checks_per_timestep;
	float m_max_velocity;
	float m_max_angular_velocity;
	float m_look_ahead_time_objects_vs_world;
	float m_look_ahead_time_objects_vs_object;
	float m_min_friction_mass;
	float m_max_friction_mass;

	void defaults() {
		this->m_max_collisions_per_object_per_timestep = 6;
		this->m_max_collision_checks_per_timestep = 250;
		this->m_max_velocity = k_max_velocity;
		this->m_max_angular_velocity = k_max_angular_velocity;
		this->m_look_ahead_time_objects_vs_world = 1.0f;
		this->m_look_ahead_time_objects_vs_object = 0.5f;
		this->m_min_friction_mass = default_min_fricion_mass;
		this->m_max_friction_mass = default_max_fricion_mass;
	}
};

enum projectile_type_t {
	TF_PROJECTILE_NONE,
	TF_PROJECTILE_BULLET,
	TF_PROJECTILE_ROCKET,
	TF_PROJECTILE_PIPEBOMB,
	TF_PROJECTILE_PIPEBOMB_REMOTE,
	TF_PROJECTILE_SYRINGE,
	TF_PROJECTILE_FLARE,
	TF_PROJECTILE_JAR,
	TF_PROJECTILE_ARROW,
	TF_PROJECTILE_FLAME_ROCKET,
	TF_PROJECTILE_JAR_MILK,
	TF_PROJECTILE_HEALING_BOLT,
	TF_PROJECTILE_ENERGY_BALL,
	TF_PROJECTILE_ENERGY_RING,
	TF_PROJECTILE_PIPEBOMB_PRACTICE,
	TF_PROJECTILE_CLEAVER,
	TF_PROJECTILE_STICKY_BALL,
	TF_PROJECTILE_CANNONBALL,
	TF_PROJECTILE_BUILDING_REPAIR_BOLT,
	TF_PROJECTILE_FESTIVE_ARROW,
	TF_PROJECTILE_THROWABLE,
	TF_PROJECTILE_SPELL,
	TF_PROJECTILE_FESTIVE_JAR,
	TF_PROJECTILE_FESTIVE_HEALING_BOLT,
	TF_PROJECTILE_BREADMONSTER_JARATE,
	TF_PROJECTILE_BREADMONSTER_MADMILK,
	TF_PROJECTILE_GRAPPLINGHOOK,
	TF_PROJECTILE_SENTRY_ROCKET,
	TF_PROJECTILE_BREAD_MONSTER,
	TF_PROJECTILE_JAR_GAS,
	TF_PROJECTILE_BALLOFFIRE,
	TF_NUM_PROJECTILES
};


class CThirdPersonManager
{
public:
	void	SetCameraOffsetAngles(const Vector& vecOffset) { m_vecCameraOffset = vecOffset; }
	const Vector& GetCameraOffsetAngles(void) const { return m_vecCameraOffset; }

	void	SetDesiredCameraOffset(const Vector& vecOffset) { m_vecDesiredCameraOffset = vecOffset; }
	const Vector& GetDesiredCameraOffset(void) const { return m_vecDesiredCameraOffset; }

	// Vector GetFinalCameraOffset(void)
	// {
	// 	Vector vDesired = GetDesiredCameraOffset();

	// 	if (m_flUpFraction != 1.0f)
	// 	{
	// 		vDesired.z += m_flUpOffset;
	// 	}

	// 	return vDesired;
	// }

	void	SetCameraOrigin(const Vector& vecOffset) { m_vecCameraOrigin = vecOffset; }
	const Vector& GetCameraOrigin(void) const { return m_vecCameraOrigin; }

	/*void	Update(void);*/

	//void	PositionCamera(CBaseEntity* pPlayer, const QAngle& angles);

	void	UseCameraOffsets(bool bUse) { m_bUseCameraOffsets = bUse; }
	bool	UsingCameraOffsets(void) { return m_bUseCameraOffsets; }

	const QAngle& GetCameraViewAngles(void) const { return m_ViewAngles; }

	/*Vector	GetDistanceFraction(void);

	bool	WantToUseGameThirdPerson(void);*/

	void	SetOverridingThirdPerson(bool bOverride) { m_bOverrideThirdPerson = bOverride; }
	bool	IsOverridingThirdPerson(void) { return m_bOverrideThirdPerson; }

	/*void	Init(void);*/

	void	SetForcedThirdPerson(bool bForced) { m_bForced = bForced; }
	bool	GetForcedThirdPerson() const { return m_bForced; }
public:
	Vector		m_vecCameraOffset;
	// Distances from the center
	Vector		m_vecDesiredCameraOffset;

	Vector m_vecCameraOrigin;

	bool	m_bUseCameraOffsets;

	QAngle	m_ViewAngles;

	float	m_flFraction;
	float	m_flUpFraction;

	float	m_flTargetFraction;
	float	m_flTargetUpFraction;

	bool	m_bOverrideThirdPerson;

	bool	m_bForced;

	float	m_flUpOffset;

	float	m_flLerpTime;
	float	m_flUpLerpTime;
};

class CClockDriftMgr
{
private:
	enum
	{
		// This controls how much it smoothes out the samples from the server.
		NUM_CLOCKDRIFT_SAMPLES = 16
	};
public:
	// This holds how many ticks the client is ahead each time we get a server tick.
	// We average these together to get our estimate of how far ahead we are.
	float m_ClockOffsets[NUM_CLOCKDRIFT_SAMPLES];
	int m_iCurClockOffset;

	int m_nServerTick;		// Last-received tick from the server.
	int	m_nClientTick;		// The client's own tick counter (specifically, for interpolation during rendering).
							// The server may be on a slightly different tick and the client will drift towards it.
};

class CClientState
{
public:
	byte pad0[0x10];
	INetChannel* m_NetChannel;			// 0x10
	byte pad1[0x140];
	CClockDriftMgr	m_ClockDriftMgr;		// 0x154
	int				m_nDeltaTick;			// 0x1A0
	byte pad2[0x110];
	int				m_nMaxClients;			// 0x2B4	
	byte pad3[0x486C];
	int				lastoutgoingcommand;	// 0x4B24
	int				chokedcommands;			// 0x4B28
	int				last_command_ack;		// 0x4B2C
	float           m_frameTime;

	int GetSignonState()
	{
		return *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(this) + 304);
	}

	int GetServerCount()
	{
		return *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(this) + 320);
	}

	void ForceFullUpdate()
	{
		using FN = void(__thiscall*)(CClientState*);
		return reinterpret_cast<FN>(g_Pattern.Find(_(L"engine.dll"), _(L"56 8B F1 83 BE ? ? ? ? ? 74 1D")))(this);
	}
};

class CSteamInterfaces
{
public:
	ISteamFriends002* Friends002 = nullptr;
	ISteamFriends015* Friends015 = nullptr;
	ISteamApps006* SteamApps = nullptr;
	ISteamUserStats011* UserStats = nullptr;
	ISteamClient017* Client = nullptr;
	ISteamUser017* User = nullptr;
	ISteamUtils007* Utils007 = nullptr;
	ISteamNetworking004* Networking = nullptr;
	ISteamNetworkingUtils* NetworkingUtils = nullptr;

	void Init();
};

namespace I
{
	inline i_physics* IPhysics = nullptr;
	inline i_physics_collision* IPhyicsCollison = nullptr;
	inline i_physics_surface_props* IPhysicsSurfaceProps = nullptr;
	inline CBaseClientDLL* BaseClientDLL = nullptr;
	inline CClientDLLSharedAppSystems* ClientDLLSharedAppSystems = nullptr;
	inline CClientState* ClientState = nullptr;
	inline CClientModeShared* ClientModeShared = nullptr;
	inline CEngineClient* EngineClient = nullptr;
	inline IVEngineEffects* EngineEffects = nullptr;
	inline CPanel* VGuiPanel = nullptr;
	inline CSurface* VGuiSurface = nullptr;
	inline CClientEntityList* ClientEntityList = nullptr;
	inline IEffects* EffectsClient = nullptr;
	inline CModelInfoClient* ModelInfoClient = nullptr;
	inline CEngineTrace* EngineTrace = nullptr;
	inline CPrediction* Prediction = nullptr;
	inline CGameMovement* GameMovement = nullptr;
	inline CMoveHelper* MoveHelper = nullptr;
	inline ICvar* Cvar = nullptr;
	inline CGlobalVarsBase* GlobalVars = nullptr;
	inline CEngineVGui* EngineVGui = nullptr;
	inline void* DemoPlayer = nullptr;
	inline IVRenderView* RenderView = nullptr;
	inline IViewRender* ViewRender = nullptr;
	inline CDebugOverlay* DebugOverlay = nullptr;
	inline CGameEventManager* GameEventManager = nullptr;
	inline CModelRender* ModelRender = nullptr;
	inline CMaterialSystem* MaterialSystem = nullptr;
	inline IInput* Input = nullptr;
	inline IKeyValuesSystem* KeyValuesSystem = nullptr;
	inline IUniformRandomStream* UniformRandomStream = nullptr;
	inline void* StudioRender = nullptr;
	inline IInputSystem* InputSystem = nullptr;
	inline void* CHud = nullptr;
	inline void* CTFGameMovement = nullptr;
	inline IAchievementMgr* AchievementMgr = nullptr;
	inline IViewRenderBeams* ViewRenderBeams = nullptr;
	inline IEngineSound* EngineSound = nullptr;

	inline void* BackpackPanel = nullptr;
	inline CTFGCClientSystem* TFGCClientSystem = nullptr;
	inline CTFPartyClient* TFPartyClient = nullptr;
	inline CTFInventoryManager* TFInventoryManager = nullptr;
	inline CThirdPersonManager* ThirdPersonManager = nullptr;
	inline IDirect3DDevice9* DirectXDevice = nullptr;
	inline ClientModeTFNormal* ClientModeTF = nullptr;

	// TODO: These do not belong here
	inline int32_t* RandomSeed = nullptr;
	inline bool* AllowSecureServers = nullptr;
}

class CInterfaces
{
public: 
	void Init();
};

inline CInterfaces g_Interfaces;
inline CSteamInterfaces g_SteamInterfaces;
