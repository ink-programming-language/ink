// Translated from solution.cpp.

func isprime(ar: dynamic) -> dynamic
{
  if ((ar == 1))
  {
    return false;
  }
  var i: dynamic = cpp_uninitialized();
  {
    i = 2;
    while (((i * i) <= ar))
    {
      if (((ar % i) == 0))
      {
        return false;
      }
      i += 1;
    }
  }
  return true;
}

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  if ((b == 0))
  {
    return a;
  }
  return gcd(b, (a % b));
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var m: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
  var v: dynamic = "";
  var z: dynamic = "";
  var a: dynamic = "";
  var b: dynamic = "";
  var p: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  read(s);
  {
    var i: dynamic = 0;
    while ((i < s.size()))
    {
      if ((s[i] != cpp_char("a")))
      {
        z = (z + s[i]);
      }
      i += 1;
    }
  }
  if ((z.size() % 2))
  {
    write(":(\n");
  } else
  {
    m = (z.size() / 2);
    k = (s.size() - m);
    v = s.substr(k, (s.size() - 1));
    a = z.substr(0, m);
    if ((v == a))
    {
      s = s.substr(0, k);
      write(s, "\n");
    } else
    {
      write(":(\n");
    }
  }
}
