/**
 * Wire identifier for a circuit part
 *
 * This acts as a "in-between" graph node for connecting potentially many
 * circuit gates to a single wire. It has a name for convenience, and a list of
 * connected gates.
 */

#include <string>

class WireId {
	public:
		/**
		 * Basic constructor
		 *
		 * @param id Id of the wire
		 * @param name Convenience name for the wire
		 */
		WireId(size_t id, const std::string& name);

		/**
		 * Id-based equality
		 */
		bool operator==(const WireId& oth) const {
			return id == oth.id;
		}

		/**
		 * Id-based comparaison
		 */
		bool operator<(const WireId& oth) const {
			return id < oth.id;
		}

	private:
		size_t id;
		std::string name;
};

