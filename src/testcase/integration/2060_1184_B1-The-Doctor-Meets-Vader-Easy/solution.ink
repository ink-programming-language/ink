// Translated from solution.cpp.

var mapi: dynamic;

var base: dynamic;

var mod = (1e9 + 7);

var sl = cpp_char("\n");

func checkmin(x: dynamic, y: dynamic)
{
  if ((y < x))
  {
    x = y;
  }
}

func checkmax(x: dynamic, y: dynamic)
{
  if ((y > x))
  {
    x = y;
  }
}

var s = 0;

var b = 0;

func compare(p: dynamic, i: dynamic)
{
  return (p.first <= i);
}

func main()
{
  read(s, b);
  base.resize(b);
  var presum = cpp_construct((b + 1));
  {
    var i = (0);
    while ((i < (s)))
    {
      read(space[i]);
      i += 1;
    }
  }
  {
    var i = (0);
    while ((i < (b)))
    {
      read(base[i].first, base[i].second);
      i += 1;
    }
  }
  sort((base).begin(), (base).end());
  presum[0] = 0;
  {
    var i = (0);
    while ((i < (b)))
    {
      presum[(i + 1)] = (presum[i] + base[i].second);
      i += 1;
    }
  }
  {
    var i = (0);
    while ((i < (s)))
    {
      var idx = (lower_bound((base).begin(), (base).end(), space[i], compare) - base.begin());
      write(presum[idx], cpp_char(" "));
      i += 1;
    }
  }
  write(sl);
  return 0;
}
