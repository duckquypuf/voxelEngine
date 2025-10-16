class Program
{
    enum GateType
    {
        INPUT,
        OUTPUT,
        AND,
        NOT
    }

    class Circuit
    {
        public string name;

        public List<Gate> gates = new List<Gate>();
        public List<Gate> inputs = new List<Gate>();
        public List<Gate> outputs = new List<Gate>();

        public List<Circuit> subCircuits = new List<Circuit>();

        public Circuit(string _name)
        {
            name = _name;
        }

        public Gate AddGate(GateType type)
        {
            Gate gate = new Gate(type);
            gates.Add(gate);

            if (type == GateType.OUTPUT)
            {
                outputs.Add(gate);
            } else if(type == GateType.INPUT)
            {
                inputs.Add(gate);
            }

            return gate;
        }

        public Circuit AddSubCircuit(Circuit subCircuit)
        {
            subCircuits.Add(subCircuit);
            return subCircuit;
        }

        public void ConnectGates(Gate from, Gate to, int inputIndex)
        {
            if (to.inputs == null)
            {
                to.inputs = new Gate[to.numInputs];
            }
            to.inputs[inputIndex] = from;
        }
        
        public Gate GetInput(int index)
        {
            if (index >= inputs.Count) throw new Exception("Input index out of range");
            return inputs[index];
        }

        public bool[] Evaluate()
        {
            bool[] results = new bool[outputs.Count];

            for (int i = 0; i < outputs.Count; i++)
            {
                results[i] = outputs[i].Evaluate();
            }

            return results;
        }
    }

    class Gate
    {
        public GateType type;
        public int numInputs;
        public int numOutputs;

        public bool inputValue;
        public Gate[]? inputs;

        public Gate(GateType _type)
        {
            type = _type;
            switch (type)
            {
                case GateType.INPUT:
                    numInputs = 0;
                    numOutputs = 1;
                    break;
                case GateType.OUTPUT:
                    numInputs = 1;
                    numOutputs = 0;
                    break;
                case GateType.AND:
                    numInputs = 2;
                    numOutputs = 1;
                    break;
                case GateType.NOT:
                    numInputs = 1;
                    numOutputs = 1;
                    break;
                default:
                    throw new Exception("Unknown gate type");
            }
        }

        public bool Evaluate()
        {
            switch (type)
            {
                case GateType.INPUT:
                    return inputValue;
                case GateType.OUTPUT:
                    return inputs![0].Evaluate();
                case GateType.AND:
                    return inputs![0].Evaluate() && inputs![1].Evaluate();
                case GateType.NOT:
                    return !inputs![0].Evaluate();
                default:
                    throw new Exception("Unknown gate type");
            }
        }
    }

    static void Main(String[] args)
    {
        bool input1, input2;

        Circuit nand = new Circuit("Nand");
        List<Gate> inputs = new List<Gate>();
        inputs.Add(nand.AddGate(GateType.INPUT));
        inputs.Add(nand.AddGate(GateType.INPUT));
        Gate and = nand.AddGate(GateType.AND);
        Gate not = nand.AddGate(GateType.NOT);
        Gate output = nand.AddGate(GateType.OUTPUT);

        nand.ConnectGates(inputs[0], and, 0);
        nand.ConnectGates(inputs[1], and, 1);
        nand.ConnectGates(and, not, 0);
        nand.ConnectGates(not, output, 0);

        Circuit or = new Circuit("Or");
        Gate orInput1 = or.AddGate(GateType.INPUT);
        Gate orInput2 = or.AddGate(GateType.INPUT);
        Gate orNot1 = or.AddGate(GateType.NOT);
        Gate orNot2 = or.AddGate(GateType.NOT);
        Circuit orNand = or.AddSubCircuit(nand);
        Gate orOutput = or.AddGate(GateType.OUTPUT);

        or.ConnectGates(orInput1, orNot1, 0);
        or.ConnectGates(orInput2, orNot2, 0);
        or.ConnectGates(orNot1, orNand.inputs![0], 0);
        or.ConnectGates(orNot2, orNand.inputs![1], 1);
        or.ConnectGates(orNand.outputs[0], orOutput, 0);

        Console.WriteLine("Output: " + orOutput.Evaluate());
    }
}
