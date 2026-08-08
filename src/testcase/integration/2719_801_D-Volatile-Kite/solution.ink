// Translated from solution.cpp.

class d
{
  func operator(x: dynamic)
  {
      write(cpp_char(" "), x);
      return (*this);
    }
  func operator(x: dynamic)
  {
      for (var x in x)
      {
        write(cpp_char(" "), x);
      }
      return (*this);
    }
}

var d_t: dynamic;

var I = cpp_construct(0, 1);

func projp(p: dynamic, a: dynamic, b: dynamic)
{
  return (a + (((conj((p - a)) * ((b - a)))).real() / conj((b - a))));
}

func reflep(p: dynamic, a: dynamic, b: dynamic)
{
  return (a + (conj((((p - a)) / ((b - a)))) * ((b - a))));
}

func rotp(a: dynamic, p: dynamic, ang: dynamic)
{
  return ((((a - p)) * polar(1.0, ang)) + p);
}

var n: dynamic;

var V: dynamic;

func ok(d: dynamic)
{
  {
    var i = cpp_construct(0);
    while ((i < n))
    {
      var u = ((((i - 1) + n)) % n);
      var v = (((i + 1)) % n);
      var p = V[i];
      var xx = projp(V[i], V[u], V[v]);
      if (((abs((p - xx)) - (1e-10)) <= (2 * d)))
      {
        return false;
      }
      i += 1;
    }
  }
  return true;
}

func main()
{
  ios_base.sync_with_stdio(false);
  read(n);
  {
    var i = cpp_construct(0);
    while ((i < n))
    {
      var a: dynamic;
      var b: dynamic;
      read(a, b);
      V.push_back(pt(a, b));
      i += 1;
    }
  }
  var l = 0;
  var r = 1e9;
  while (((fabs((l - r)) + (1e-10)) > 1e-6))
  {
    var m = (((l + r)) / 2);
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
