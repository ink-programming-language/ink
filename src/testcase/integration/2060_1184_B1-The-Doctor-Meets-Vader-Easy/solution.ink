// Translated from solution.cpp.

var mapi: dynamic = cpp_uninitialized();

var base: dynamic = cpp_uninitialized();

var mod: dynamic = (1e9 + 7);

var sl: dynamic = cpp_char("\n");

func checkmin(x: dynamic, y: dynamic) -> dynamic
{
  if ((y < x))
  {
    x = y;
  }
}

func checkmax(x: dynamic, y: dynamic) -> dynamic
{
  if ((y > x))
  {
    x = y;
  }
}

var s: dynamic = 0;

var b: dynamic = 0;

func compare(p: dynamic, i: dynamic) -> dynamic
{
  return (p.first <= i);
}

func main() -> dynamic
{
  read(s, b);
  base.resize(b);
  var presum: dynamic = cpp_construct((b + 1));
  {
    var i: dynamic = (0);
    while ((i < (s)))
    {
      read(space[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = (0);
    while ((i < (b)))
    {
      read(base[i].first, base[i].second);
      i += 1;
    }
  }
  sort((base).begin(), (base).end());
  presum[0] = 0;
  {
    var i: dynamic = (0);
    while ((i < (b)))
    {
      presum[(i + 1)] = (presum[i] + base[i].second);
      i += 1;
    }
  }
  {
    var i: dynamic = (0);
    while ((i < (s)))
    {
      var idx: dynamic = (lower_bound((base).begin(), (base).end(), space[i], compare) - base.begin());
      write(presum[idx], cpp_char(" "));
      i += 1;
    }
  }
  write(sl);
  return 0;
}
