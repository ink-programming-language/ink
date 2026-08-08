// Translated from solution.cpp.

var mem = cpp_array(5001, 51);

var x: dynamic;

var y: dynamic;

var z: dynamic;

var v = cpp_array(100);

var used = cpp_array(5001, 51);

class po
{
  var e: dynamic;
  var a: dynamic;
}

var E = cpp_array(101);

func dfs(pos: dynamic, mo: dynamic)
{
  if ((pos == y))
  {
    return mo;
  }
  if (used[pos][mo])
  {
    return mem[pos][mo];
  }
  used[pos][mo] = 1;
  {
    var i = 0;
    while ((i < x))
    {
      var npos = min(y, (pos + v[i]));
      var nmo = mo;
      var e = E[npos].e;
      var a = E[npos].a;
      if (e)
      {
        if ((e == 1))
        {
          npos = min(y, (npos + a));
        }
        if ((e == 2))
        {
          nmo += a;
        }
        if ((e == 3))
        {
          nmo = max(0, (nmo - a));
        }
      }
      mem[pos][mo] += (dfs(npos, nmo) / x);
      i += 1;
    }
  }
  return mem[pos][mo];
}

func main()
{
  while (1)
  {
    read(x, y, z);
    if ((((!x) && (!y)) && (!z)))
    {
      break;
    }
    {
      var i = 0;
      while ((i < x))
      {
        read(v[i]);
        i += 1;
      }
    }
    memset(E, 0, cpp_sizeof((E)));
    {
      var i = 0;
      var a: dynamic;
      while ((i < z))
      {
        read(a);
        read(E[a].e, E[a].a);
        i += 1;
      }
    }
    memset(used, 0, cpp_sizeof((used)));
    {
      var i = 0;
      while ((i < 51))
      {
        {
          var j = 0;
          while ((j < 5001))
          {
            mem[i][j] = 0;
            j += 1;
          }
        }
        i += 1;
      }
    }
    write(cpp_cast(dfs(0, 0)), "\n");
  }
  return 0;
}
