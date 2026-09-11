// Translated from solution.cpp.

var d: dynamic = cpp_uninitialized();

var z: dynamic = 26;

var s: dynamic = cpp_uninitialized();

func score(t: dynamic) -> dynamic
{
  var sum: dynamic = 0;
  var last: dynamic = cpp_construct(z, -1);
  {
    var i: dynamic = 0;
    while ((i < t.size()))
    {
      sum += s[i][t[i]];
      last[t[i]] = i;
      {
        var j: dynamic = 0;
        while ((j < z))
        {
          sum -= (c[j] * ((((i + 1)) - ((last[j] + 1)))));
          j += 1;
        }
      }
      i += 1;
    }
  }
  return sum;
}

func Main() -> dynamic
{
  read(d);
  s = vector(d, vector(z));
  {
    var i: dynamic = 0;
    while ((i < z))
    {
      read(c[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < d))
    {
      {
        var j: dynamic = 0;
        while ((j < z))
        {
          read(s[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var t: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < d))
    {
      var tmp: dynamic = cpp_uninitialized();
      read(tmp);
      t.push_back((tmp - 1));
      i += 1;
    }
  }
  var m: dynamic = cpp_uninitialized();
  read(m);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      var d: dynamic = cpp_uninitialized();
      var q: dynamic = cpp_uninitialized();
      read(d, q);
      d -= 1;
      q -= 1;
      t[d] = q;
      write(score(t), "\n");
      i += 1;
    }
  }
}

func main(argc: dynamic, argv: dynamic) -> dynamic
{
  Main();
  return 0;
}
