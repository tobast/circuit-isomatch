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
class WireManager;
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
         * @param manager the `WireManager` used to create this wire
		 */
		WireId(size_t id, const std::string& name, WireManager* manager);

        ~WireId();

		/** Id-based equality */
		bool operator==(const WireId& oth) const;

		/** Id-based equality */
		bool operator==(WireId& oth);

		/** Id-based comparaison */
		bool operator<(const WireId& oth) const;

		/** Id-based comparaison */
		bool operator<(WireId& oth);

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
        const std::vector<CircuitTree*>& connectedCirc();

        /** Get the list of wires connected to that wire. Fast. */
        const std::vector<PinConnection>& connectedPins();

        /** Get the list of circuits connected to that wire, possibly through
         * other wires. Must perform a DFS through connected wires and create
         * the list on-the-fly, which might be a bit slow for heavy use. */
        std::vector<CircuitTree*> connected();

        /** Get the name of this wire */
        const std::string& name() { return inner()->name; }

        /** Get this wire's display unique name */
        std::string uniqueName();

	private:

        void walkConnected(std::unordered_set<CircuitTree*>& curConnected,
                std::unordered_set<WireId>& seenWires,
                WireId* curWire);

        struct Inner {
            size_t id;
            std::string name;
            WireManager* manager;
            std::vector<CircuitTree*> connected;
            std::vector<PinConnection> connectedPins;
        };

        void merge(WireId* other);
        void rename(const std::string& nName) { inner()->name = nName; }

        WireId* ufRoot();
        inline Inner* inner() { return ufRoot()->end; };
        const Inner* inner() const;

        union {
            Inner* end;
            WireId* chain;
        };
        bool isEndpoint;
        unsigned short ufDepth;

    friend WireManager; // set the wire's name
};

namespace std {
    template<> struct hash<WireId> {
        typedef WireId argument_type;
        typedef std::size_t result_type;
        result_type operator()(const argument_type& wire) const {
            return wire.inner()->id;
        }
    };
}

