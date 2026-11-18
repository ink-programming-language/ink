using System;

// Initial conditions and scalar algorithm follow Christoph Bauer's Benchmarks Game program.
// See LICENSE.benchmarksgame for the upstream redistribution notice.
internal static class NBody
{
    private const double SolarMass = 4 * 3.141592653589793 * 3.141592653589793;
    private const double DaysPerYear = 365.24;
    private const double TimeStep = 0.01;

    private struct Body
    {
        internal double X;
        internal double Y;
        internal double Z;
        internal double VX;
        internal double VY;
        internal double VZ;
        internal double Mass;

        internal Body(double x, double y, double z, double vx, double vy, double vz, double mass)
        {
            X = x;
            Y = y;
            Z = z;
            VX = vx;
            VY = vy;
            VZ = vz;
            Mass = mass;
        }
    }

    private struct Simulation
    {
        internal readonly Body[] Bodies;
        internal readonly double InitialEnergy;
        internal readonly double FinalEnergy;

        internal Simulation(Body[] bodies, double initialEnergy, double finalEnergy)
        {
            Bodies = bodies;
            InitialEnergy = initialEnergy;
            FinalEnergy = finalEnergy;
        }
    }

    private static Body[] InitialBodies()
    {
        return new Body[]
        {
            new Body(0, 0, 0, 0, 0, 0, SolarMass),
            new Body(4.84143144246472090, -1.16032004402742839, -0.103622044471123109, 0.00166007664274403694 * DaysPerYear, 0.00769901118419740425 * DaysPerYear, -0.0000690460016972063023 * DaysPerYear, 0.000954791938424326609 * SolarMass),
            new Body(8.34336671824457987, 4.12479856412430479, -0.403523417114321381, -0.00276742510726862411 * DaysPerYear, 0.00499852801234917238 * DaysPerYear, 0.0000230417297573763929 * DaysPerYear, 0.000285885980666130812 * SolarMass),
            new Body(12.8943695621391310, -15.1111514016986312, -0.223307578892655734, 0.00296460137564761618 * DaysPerYear, 0.00237847173959480950 * DaysPerYear, -0.0000296589568540237556 * DaysPerYear, 0.0000436624404335156298 * SolarMass),
            new Body(15.3796971148509165, -25.9193146099879641, 0.179258772950371181, 0.00268067772490389322 * DaysPerYear, 0.00162824170038242295 * DaysPerYear, -0.0000951592254519715870 * DaysPerYear, 0.0000515138902046611451 * SolarMass),
        };
    }

    private static void OffsetMomentum(Body[] bodies)
    {
        double px = 0;
        double py = 0;
        double pz = 0;
        foreach (Body body in bodies)
        {
            px += body.VX * body.Mass;
            py += body.VY * body.Mass;
            pz += body.VZ * body.Mass;
        }
        bodies[0].VX = -px / SolarMass;
        bodies[0].VY = -py / SolarMass;
        bodies[0].VZ = -pz / SolarMass;
    }

    private static double Energy(Body[] bodies)
    {
        double energy = 0;
        for (int i = 0; i < bodies.Length; ++i)
        {
            ref Body left = ref bodies[i];
            energy += 0.5 * left.Mass * (left.VX * left.VX + left.VY * left.VY + left.VZ * left.VZ);
            for (int j = i + 1; j < bodies.Length; ++j)
            {
                ref Body right = ref bodies[j];
                double dx = left.X - right.X;
                double dy = left.Y - right.Y;
                double dz = left.Z - right.Z;
                double distance = Math.Sqrt(dx * dx + dy * dy + dz * dz);
                energy -= left.Mass * right.Mass / distance;
            }
        }
        return energy;
    }

    private static void Advance(Body[] bodies)
    {
        for (int i = 0; i < bodies.Length; ++i)
        {
            ref Body left = ref bodies[i];
            for (int j = i + 1; j < bodies.Length; ++j)
            {
                ref Body right = ref bodies[j];
                double dx = left.X - right.X;
                double dy = left.Y - right.Y;
                double dz = left.Z - right.Z;
                double distance = Math.Sqrt(dx * dx + dy * dy + dz * dz);
                double magnitude = TimeStep / (distance * distance * distance);
                left.VX -= dx * right.Mass * magnitude;
                left.VY -= dy * right.Mass * magnitude;
                left.VZ -= dz * right.Mass * magnitude;
                right.VX += dx * left.Mass * magnitude;
                right.VY += dy * left.Mass * magnitude;
                right.VZ += dz * left.Mass * magnitude;
            }
        }
        for (int i = 0; i < bodies.Length; ++i)
        {
            ref Body body = ref bodies[i];
            body.X += TimeStep * body.VX;
            body.Y += TimeStep * body.VY;
            body.Z += TimeStep * body.VZ;
        }
    }

    private static Simulation Simulate(int steps)
    {
        Body[] bodies = InitialBodies();
        OffsetMomentum(bodies);
        double initialEnergy = Energy(bodies);
        for (int step = 0; step < steps; ++step)
        {
            Advance(bodies);
        }
        return new Simulation(bodies, initialEnergy, Energy(bodies));
    }

    private static double[] BodyValues(Body[] bodies)
    {
        double[] values = new double[bodies.Length * 7];
        for (int i = 0; i < bodies.Length; ++i)
        {
            ref Body body = ref bodies[i];
            int offset = i * 7;
            values[offset] = body.X;
            values[offset + 1] = body.Y;
            values[offset + 2] = body.Z;
            values[offset + 3] = body.VX;
            values[offset + 4] = body.VY;
            values[offset + 5] = body.VZ;
            values[offset + 6] = body.Mass;
        }
        return values;
    }

    private static bool NearlyEqual(double actual, double expected)
    {
        return !double.IsNaN(actual) && !double.IsInfinity(actual) && !double.IsNaN(expected) && !double.IsInfinity(expected)
            && Math.Abs(actual - expected) <= 1e-12 * Math.Max(1.0, Math.Abs(expected));
    }

    private static bool Matches(Simulation actual, Simulation expected, double[] expectedBodies)
    {
        if (!NearlyEqual(actual.InitialEnergy, expected.InitialEnergy) || !NearlyEqual(actual.FinalEnergy, expected.FinalEnergy))
        {
            return false;
        }

        double[] bodyValues = BodyValues(actual.Bodies);
        for (int i = 0; i < expectedBodies.Length; ++i)
        {
            if (!NearlyEqual(bodyValues[i], expectedBodies[i]))
            {
                return false;
            }
        }
        return true;
    }

    private static int Main(string[] args)
    {
        if (!Benchmark.ReadArguments(args, 100000, 100000000, out int steps, out int samples, out int warmup))
        {
            Console.Error.WriteLine("Usage: nbody.exe [STEPS=100000 (0..100000000)] [SAMPLES=11 (1..1000)] [WARMUP=3 (0..1000)]");
            return 2;
        }

        Simulation expected = Simulate(steps);
        double[] expectedBodies = BodyValues(expected.Bodies);
        if (!Benchmark.Measure(steps, samples, warmup, Simulate, value => Matches(value, expected, expectedBodies), out Simulation actual, out double[] timings))
        {
            Console.Error.WriteLine("n-body simulation did not match the reference result");
            return 1;
        }

        Console.WriteLine("{{\"runtime\":\"C#\",\"steps\":{0},\"initial_energy\":{1},\"final_energy\":{2},\"bodies\":[{3}],\"samples_ms\":[{4}],\"warmup\":{5}}}", steps, Benchmark.Number(actual.InitialEnergy), Benchmark.Number(actual.FinalEnergy), Benchmark.Numbers(BodyValues(actual.Bodies)), Benchmark.Numbers(timings), warmup);
        return 0;
    }
}
