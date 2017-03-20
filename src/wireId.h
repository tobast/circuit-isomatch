/**
 * Wire identifier for a circuit part
 *
 * This acts as a "in-between" graph node for connecting potentially many
 * circuit gates to a single wire. It has a name for convenience, and a list of
 * connected gates.
 */

#pragma once
#include <string>
#include <vector>
#include <unordered_set>

// Circular inclusion
class CircuitTree;
class CircuitGroup;
class IOPin;

class WireId {
    friend struct std::hash<WireId>;
	public:
        /** Connection to an IO pin */
        struct PinConnection {
            PinConnection(IOPin* pin, WireId* other) :
                pin(pin), other(other) {}
            IOPin* pin;
            WireId* other;
        };

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

        /** Connect a circuit to this wire. Should be handled by circuit
         * classes silently.
         */
        void connect(CircuitTree* circ);

        /** Connect a pin to this wire. */
        void connect(const PinConnection& pin);

        /** Connect a pin to this wire, creating a `PinConnection` on the fly.
         */
        void connect(IOPin* pin, WireId* other);

        /** Get the list of circuits connected to that wire. Fast. */
        const std::vector<CircuitTree*>& connectedCirc() const;

        /** Get the list of wires connected to that wire. Fast. */
        const std::vector<PinConnection>& connectedPins() const;

        /** Get the list of circuits connected to that wire, possibly through
         * other wires. Must perform a DFS through connected wires and create
         * the list on-the-fly, which might be a bit slow for heavy use. */
        std::vector<CircuitTree*> connected() const;

	private:
        void walkConnected(std::unordered_set<CircuitTree*>& curConnected,
                std::unordered_set<WireId>& seenWires,
                const WireId* curWire) const;

		size_t id;
		std::string name;
        std::vector<CircuitTree*> _connected;
        std::vector<PinConnection> _connectedPins;
};

namespace std {
    template<> struct hash<WireId> {
        typedef WireId argument_type;
        typedef std::size_t result_type;
        result_type operator()(argument_type const& wire) const {
            return wire.id;
        }
    };
}

