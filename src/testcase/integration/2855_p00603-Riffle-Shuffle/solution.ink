// Translated from solution.cpp.

var Q = cpp_array(3);

var p: dynamic;

var q: dynamic;

func solve(a: dynamic)
{
  {
    var i = 0;
    while ((i < 1000))
    {
      if ((Q[(i % 2)].size() < a))
      {
        while ((!Q[(i % 2)].empty()))
        {
          Q[2].push(Q[(i % 2)].front());
          Q[(i % 2)].pop();
        }
      } else
      {
        {
          var j = 0;
          while ((j < a))
          {
            Q[2].push(Q[(i % 2)].front());
            Q[(i % 2)].pop();
            j += 1;
          }
        }
      }
      i += 1;
    }
  }
}

func main()
{
  while (((cin >> p) >> q))
  {
    {
      var i = 0;
      while ((i < (p / 2)))
      {
        Q[1].push(i);
        i += 1;
      }
    }
    {
      var i = (p / 2);
      while ((i < p))
      {
        Q[0].push(i);
        i += 1;
      }
    }
    {
      var i = 0;
      while ((i < q))
      {
        if ((i >= 1))
        {
          var V = Q[2].size();
          {
            var j = 0;
            while ((j < (V / 2)))
            {
              Q[1].push(Q[2].front());
              Q[2].pop();
              j += 1;
            }
          }
          {
            var j = (V / 2);
            while ((j < V))
            {
              Q[0].push(Q[2].front());
              Q[2].pop();
              j += 1;
            }
          }
        }
        var a: dynamic;
        read(a);
        solve(a);
        i += 1;
      }
    }
    while ((Q[2].size() >= 2))
    {
      Q[2].pop();
    }
    write(Q[2].front(), "\n");
    Q[2].pop();
  }
  return 0;
}
