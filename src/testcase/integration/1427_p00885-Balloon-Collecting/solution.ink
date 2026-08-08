// Translated from solution.cpp.

var v: dynamic;

var ok = 100000000;

var ng = -1;

func msearch(i: dynamic, p: dynamic, bn: dynamic, t: dynamic, x: dynamic, move: dynamic)
{
  if ((move < ok))
  {
    if ((i == v.size()))
    {
      if ((ok > (move + x)))
      {
        ok = (move + x);
      }
    } else if ((x == 0))
    {
      if (((t + v[i].first) > v[i].second))
      {
        if ((ng < p))
        {
          ng = p;
        }
      } else
      {
        t = v[i].second;
        x = v[i].first;
        move += v[i].first;
        msearch((i + 1), (p + 1), (bn + 1), t, x, move);
      }
    } else
    {
      if (((((abs((x - v[i].first)) * ((bn + 1))) + t) <= v[i].second) && (bn < 3)))
      {
        msearch((i + 1), (p + 1), (bn + 1), v[i].second, v[i].first, (move + abs((x - v[i].first))));
      }
      if ((((t + (x * ((bn + 1)))) + v[i].first) > v[i].second))
      {
        if ((ng < p))
        {
          ng = p;
        }
      } else
      {
        msearch((i + 1), (p + 1), 1, v[i].second, v[i].first, ((move + x) + v[i].first));
      }
    }
  }
}

func main(argument_0: dynamic)
{
  var n: dynamic;
  while (cpp_comma((cin >> n), n))
  {
    v.resize(n);
    for (var a in v)
    {
      read(a.first, a.second);
    }
    msearch(0, 1, 0, 0, 0, cpp_cast(0));
    if ((ok == 100000000))
    {
      write("NG ", ng, "\n");
    } else
    {
      write("OK ", ok, "\n");
    }
    v.clear();
    ok = 100000000;
    ng = -1;
  }
}
