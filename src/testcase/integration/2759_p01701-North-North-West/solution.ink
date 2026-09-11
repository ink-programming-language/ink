// Translated from solution.cpp.

class Fraction
{
  var numerator: dynamic = cpp_uninitialized();
  var denominator: dynamic = cpp_uninitialized();
}

func operator_shift_left(os: dynamic, f: dynamic) -> dynamic
{
  reduce(f);
  if ((f.numerator == 0))
  {
    (os << "0");
  } else if ((f.denominator == 1))
  {
    (os << f.numerator);
  } else
  {
    (((os << f.numerator) << "/") << f.denominator);
  }
  return os;
}

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  return ( ((b == 0)) ? a : gcd(b, (a % b)));
}

func lcm(a: dynamic, b: dynamic) -> dynamic
{
  return ((a * b) / gcd(a, b));
}

func reduce(f: dynamic) -> dynamic
{
  var x: dynamic = gcd(f.numerator, f.denominator);
  f.numerator /= x;
  f.denominator /= x;
}

func sub(f: dynamic, num: dynamic, den: dynamic) -> dynamic
{
  var g: dynamic = [num, den];
  reduce(g);
  var nd: dynamic = lcm(f.denominator, g.denominator);
  f.numerator *= ((nd / f.denominator));
  g.numerator *= ((nd / g.denominator));
  f.denominator = cpp_assign(g.denominator, "=", nd);
  f.numerator -= g.numerator;
  return f;
}

func add(f: dynamic, num: dynamic, den: dynamic) -> dynamic
{
  var g: dynamic = [num, den];
  reduce(g);
  var nd: dynamic = lcm(f.denominator, g.denominator);
  f.numerator *= ((nd / f.denominator));
  g.numerator *= ((nd / g.denominator));
  f.denominator = cpp_assign(g.denominator, "=", nd);
  f.numerator += g.numerator;
  return f;
}

func main() -> dynamic
{
  var in_cpp: dynamic = cpp_uninitialized();
  while (cpp_comma((cin >> in_cpp), (in_cpp != "#")))
  {
    var s: dynamic = cpp_uninitialized();
    var size: dynamic = 0;
    {
      var i: dynamic = 0;
      while ((i < cpp_cast(in_cpp.size())))
      {
        if ((in_cpp[i] == cpp_char("n")))
        {
          s += cpp_char("n");
          i += 5;
        } else
        {
          s += cpp_char("w");
          i += 4;
        }
        size += 1;
      }
    }
    reverse(s.begin(), s.end());
    var f: dynamic = cpp_uninitialized();
    var n: dynamic = 1;
    if ((s[0] == cpp_char("n")))
    {
      f.numerator = 0;
      f.denominator = 1;
    } else
    {
      f.numerator = 90;
      f.denominator = 1;
    }
    {
      var i: dynamic = 1;
      while ((i < size))
      {
        if ((s[i] == cpp_char("n")))
        {
          f = sub(f, 90, cpp_cast(pow(2.0, n)));
        } else
        {
          f = add(f, 90, cpp_cast(pow(2.0, n)));
        }
        reduce(f);
        i += 1;
        n += 1;
      }
    }
    write(f, "\n");
  }
  return 0;
}
