// Translated from solution.cpp.

func isprime(ar: dynamic)
{
  if ((ar == 1))
  {
    return false;
  }
  var i: dynamic;
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

func gcd(a: dynamic, b: dynamic)
{
  if ((b == 0))
  {
    return a;
  }
  return gcd(b, (a % b));
}

func main()
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var m: dynamic;
  var k: dynamic;
  var s: dynamic;
  var v = "";
  var z = "";
  var a = "";
  var b = "";
  var p: dynamic;
  var t: dynamic;
  read(s);
  {
    var i = 0;
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
