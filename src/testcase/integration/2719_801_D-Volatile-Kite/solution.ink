// Translated from solution.cpp.

class d
{
  func operator(x: dynamic) -> dynamic
  {
      write(cpp_char(" "), x);
      return (*self);
    }
  func operator(x: dynamic) -> dynamic
  {
      for (var x: dynamic in x)
      {
        write(cpp_char(" "), x);
      }
      return (*self);
    }
}

var d_t: dynamic = cpp_uninitialized();

var I: dynamic = cpp_construct(0, 1);

func projp(p: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  return (a + (((conj((p - a)) * ((b - a)))).real() / conj((b - a))));
}

func reflep(p: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  return (a + (conj((((p - a)) / ((b - a)))) * ((b - a))));
}

func rotp(a: dynamic, p: dynamic, ang: dynamic) -> dynamic
{
  return ((((a - p)) * polar(1.0, ang)) + p);
}

var n: dynamic = cpp_uninitialized();

var V: dynamic = cpp_uninitialized();

func ok(d: dynamic) -> dynamic
{
  {
    var i: dynamic = cpp_construct(0);
    while ((i < n))
    {
      var u: dynamic = ((((i - 1) + n)) % n);
      var v: dynamic = (((i + 1)) % n);
      var p: dynamic = V[i];
      var xx: dynamic = projp(V[i], V[u], V[v]);
      if (((abs((p - xx)) - (1e-10)) <= (2 * d)))
      {
        return false;
      }
      i += 1;
    }
  }
  return true;
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  read(n);
  {
    var i: dynamic = cpp_construct(0);
    while ((i < n))
    {
      var a: dynamic = cpp_uninitialized();
      var b: dynamic = cpp_uninitialized();
      read(a, b);
      V.push_back(pt(a, b));
      i += 1;
    }
  }
  var l: dynamic = 0;
  var r: dynamic = 1e9;
  while (((fabs((l - r)) + (1e-10)) > 1e-6))
  {
    var m: dynamic = (((l + r)) / 2);
    if (ok(m))
    {
      l = m;
    } else
    {
      r = m;
    }
  }
  write(fixed, setprecision(8), r, "\n");
  return 0;
}
