#pragma once

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstddef>
#include <cstdint>

// Initial conditions and scalar algorithm follow the Computer Language Benchmarks Game's
// n-body program contributed by Christoph Bauer:
// https://benchmarksgame-team.pages.debian.net/benchmarksgame/program/nbody-gcc-1.html
// See LICENSE.benchmarksgame for the upstream redistribution notice.
namespace ink::benchmark::nbody
{
    inline constexpr double SolarMass = 4 * 3.141592653589793 * 3.141592653589793;
    inline constexpr double DaysPerYear = 365.24;
    inline constexpr double TimeStep = 0.01;

    struct Body
    {
            double X;
            double Y;
            double Z;
            double VX;
            double VY;
            double VZ;
            double Mass;
    };

    using Bodies = std::array<Body, 5>;
    using BodyWords = std::array<double, 35>;

    static_assert(sizeof(Body) == 7 * sizeof(double));
    static_assert(sizeof(Bodies) == sizeof(BodyWords));

    struct Result
    {
            Bodies BodyValues;
            double InitialEnergy;
            double FinalEnergy;
    };

    inline Bodies initialBodies()
    {
        return {{
            {0, 0, 0, 0, 0, 0, SolarMass},
            {4.84143144246472090, -1.16032004402742839, -0.103622044471123109, 0.00166007664274403694 * DaysPerYear, 0.00769901118419740425 * DaysPerYear, -0.0000690460016972063023 * DaysPerYear, 0.000954791938424326609 * SolarMass},
            {8.34336671824457987, 4.12479856412430479, -0.403523417114321381, -0.00276742510726862411 * DaysPerYear, 0.00499852801234917238 * DaysPerYear, 0.0000230417297573763929 * DaysPerYear, 0.000285885980666130812 * SolarMass},
            {12.8943695621391310, -15.1111514016986312, -0.223307578892655734, 0.00296460137564761618 * DaysPerYear, 0.00237847173959480950 * DaysPerYear, -0.0000296589568540237556 * DaysPerYear, 0.0000436624404335156298 * SolarMass},
            {15.3796971148509165, -25.9193146099879641, 0.179258772950371181, 0.00268067772490389322 * DaysPerYear, 0.00162824170038242295 * DaysPerYear, -0.0000951592254519715870 * DaysPerYear, 0.0000515138902046611451 * SolarMass},
        }};
    }

    inline void offsetMomentum(Bodies& Values)
    {
        double PX = 0;
        double PY = 0;
        double PZ = 0;
        for (const auto& Value : Values)
        {
            PX += Value.VX * Value.Mass;
            PY += Value.VY * Value.Mass;
            PZ += Value.VZ * Value.Mass;
        }
        Values[0].VX = -PX / SolarMass;
        Values[0].VY = -PY / SolarMass;
        Values[0].VZ = -PZ / SolarMass;
    }

    inline double energy(const Bodies& Values)
    {
        double Energy = 0;
        for (std::size_t I = 0; I < Values.size(); ++I)
        {
            const auto& Left = Values[I];
            Energy += 0.5 * Left.Mass * (Left.VX * Left.VX + Left.VY * Left.VY + Left.VZ * Left.VZ);
            for (auto J = I + 1; J < Values.size(); ++J)
            {
                const auto& Right = Values[J];
                const auto DX = Left.X - Right.X;
                const auto DY = Left.Y - Right.Y;
                const auto DZ = Left.Z - Right.Z;
                const auto Distance = std::sqrt(DX * DX + DY * DY + DZ * DZ);
                Energy -= Left.Mass * Right.Mass / Distance;
            }
        }
        return Energy;
    }

    inline void advance(Bodies& Values)
    {
        for (std::size_t I = 0; I < Values.size(); ++I)
        {
            auto& Left = Values[I];
            for (auto J = I + 1; J < Values.size(); ++J)
            {
                auto& Right = Values[J];
                const auto DX = Left.X - Right.X;
                const auto DY = Left.Y - Right.Y;
                const auto DZ = Left.Z - Right.Z;
                const auto Distance = std::sqrt(DX * DX + DY * DY + DZ * DZ);
                const auto Magnitude = TimeStep / (Distance * Distance * Distance);
                Left.VX -= DX * Right.Mass * Magnitude;
                Left.VY -= DY * Right.Mass * Magnitude;
                Left.VZ -= DZ * Right.Mass * Magnitude;
                Right.VX += DX * Left.Mass * Magnitude;
                Right.VY += DY * Left.Mass * Magnitude;
                Right.VZ += DZ * Left.Mass * Magnitude;
            }
        }
        for (auto& Value : Values)
        {
            Value.X += TimeStep * Value.VX;
            Value.Y += TimeStep * Value.VY;
            Value.Z += TimeStep * Value.VZ;
        }
    }

    inline Result reference(std::uint64_t Steps)
    {
        auto Values = initialBodies();
        offsetMomentum(Values);
        const auto InitialEnergy = energy(Values);
        for (std::uint64_t Step = 0; Step < Steps; ++Step)
        {
            advance(Values);
        }
        const auto FinalEnergy = energy(Values);
        return {Values, InitialEnergy, FinalEnergy};
    }

    inline BodyWords bodyWords(const Bodies& Values)
    {
        return std::bit_cast<BodyWords>(Values);
    }

    inline bool nearlyEqual(double Actual, double Expected)
    {
        return std::isfinite(Actual) && std::isfinite(Expected) && std::abs(Actual - Expected) <= 1.0e-12 * std::max(1.0, std::abs(Expected));
    }
} // namespace ink::benchmark::nbody
