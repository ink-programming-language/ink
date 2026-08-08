// Translated from solution.cpp.

var d: dynamic;

var z = 26;

var s: dynamic;

func score(t: dynamic)
{
  var sum = 0;
  var last = cpp_construct(z, -1);
  {
    var i = 0;
    while ((i < t.size()))
    {
      sum += s[i][t[i]];
      last[t[i]] = i;
      {
        var j = 0;
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

func Main()
{
  read(d);
  s = vector(d, vector(z));
  {
    var i = 0;
    while ((i < z))
    {
      read(c[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < d))
    {
      {
        var j = 0;
        while ((j < z))
        {
          read(s[i][j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  var t: dynamic;
  {
    var i = 0;
    while ((i < d))
    {
      var tmp: dynamic;
      read(tmp);
      t.push_back((tmp - 1));
      i += 1;
    }
  }
  var m: dynamic;
  read(m);
  {
    var i = 0;
    while ((i < m))
    {
      var d: dynamic;
      var q: dynamic;
      read(d, q);
      d -= 1;
      q -= 1;
      t[d] = q;
      write(score(t), "\n");
      i += 1;
    }
  }
}

func main(argc: dynamic, argv: dynamic)
{
  Main();
  return 0;
}
