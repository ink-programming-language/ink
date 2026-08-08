// Translated from solution.cpp.

class Fraction
{
  var numerator: dynamic;
  var denominator: dynamic;
}

func operator_shift_left(os: dynamic, f: dynamic)
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

func gcd(a: dynamic, b: dynamic)
{
  return (if ((b == 0)) a else gcd(b, (a % b)));
}

func lcm(a: dynamic, b: dynamic)
{
  return ((a * b) / gcd(a, b));
}

func reduce(f: dynamic)
{
  var x = gcd(f.numerator, f.denominator);
  f.numerator /= x;
  f.denominator /= x;
}

func sub(f: dynamic, num: dynamic, den: dynamic)
{
  var g = [num, den];
  reduce(g);
  var nd = lcm(f.denominator, g.denominator);
  f.numerator *= ((nd / f.denominator));
  g.numerator *= ((nd / g.denominator));
  f.denominator = cpp_assign(g.denominator, "=", nd);
  f.numerator -= g.numerator;
  return f;
}

func add(f: dynamic, num: dynamic, den: dynamic)
{
  var g = [num, den];
  reduce(g);
  var nd = lcm(f.denominator, g.denominator);
  f.numerator *= ((nd / f.denominator));
  g.numerator *= ((nd / g.denominator));
  f.denominator = cpp_assign(g.denominator, "=", nd);
  f.numerator += g.numerator;
  return f;
}

func main()
{
  var in_cpp: dynamic;
  while (cpp_comma((cin >> in_cpp), (in_cpp != "#")))
  {
    var s: dynamic;
    var size = 0;
    {
      var i = 0;
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
    var f: dynamic;
    var n = 1;
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
      var i = 1;
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
