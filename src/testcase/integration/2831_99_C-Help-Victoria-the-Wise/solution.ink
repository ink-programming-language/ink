// Translated from solution.cpp.

var gr = ["123456", "234156", "341256", "412356", "143265", "432165", "321465", "214365", "546213", "462513", "625413", "254613", "452631", "526431", "264531", "645231", "351624", "516324", "163524", "635124", "361542", "615342", "153642", "536142"];

func pr(s: dynamic, k: dynamic)
{
  var ret = "123456";
  {
    var i = 0;
    while ((i < 6))
    {
      ret[i] = s[(gr[k][i] - cpp_char("1"))];
      i += 1;
    }
  }
  return ret;
}

func eq(s1: dynamic, s2: dynamic)
{
  {
    var i = 0;
    while ((i < 24))
    {
      if ((s1 == pr(s2, i)))
      {
        return true;
      }
      i += 1;
    }
  }
  return false;
}

var r: dynamic;

var s: dynamic;

var used = [0];

func check(x: dynamic)
{
  {
    var i = 0;
    while ((i < r.size()))
    {
      if (eq(x, r[i]))
      {
        return;
      }
      i += 1;
    }
  }
  r.push_back(x);
}

func rec(x: dynamic)
{
  if ((x.length() > 6))
  {
    return;
  }
  if ((x.length() == 6))
  {
    check(x);
    return;
  }
  {
    var i = 0;
    while ((i < 6))
    {
      if ((!used[i]))
      {
        used[i] = true;
        rec((x + s[i]));
        used[i] = false;
      }
      i += 1;
    }
  }
}

func main()
{
  read(s);
  rec("");
  write(r.size(), "\n");
  return 0;
}
