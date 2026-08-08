// Translated from solution.cpp.

var MAX = cpp_expression("#inclu");

var t = cpp_array(MAX);

var d = cpp_array(MAX);

var N: dynamic;

var Q: dynamic;

var s: dynamic;

func mycheck(goal: dynamic, place: dynamic)
{
  {
    var i = 1;
    while ((i <= Q))
    {
      if ((t[i] == s[(place - 1)]))
      {
        if ((d[i] == cpp_char("L")))
        {
          place -= 1;
        } else
        {
          place += 1;
        }
      }
      if ((place == goal))
      {
        return true;
      }
      if (((place == 0) || (place == (N + 1))))
      {
        return false;
      }
      i += 1;
    }
  }
  return false;
}

func main()
{
  read(N, Q, s);
  {
    var i = 1;
    while ((i <= Q))
    {
      read(t[i], d[i]);
      i += 1;
    }
  }
  var s = 0;
  var e = (N + 1);
  var ans0: dynamic;
  var ansN: dynamic;
  var ans: dynamic;
  var h: dynamic;
  h = (((s + e)) / 2);
  while (true)
  {
    if (mycheck(0, h))
    {
      s = h;
    } else
    {
      e = h;
    }
    h = (((s + e)) / 2);
    if (((s + 1) == e))
    {
      break;
    }
  }
  ans0 = s;
  s = 0;
  e = (N + 1);
  h = (((s + e)) / 2);
  while (true)
  {
    if (mycheck((N + 1), h))
    {
      e = h;
    } else
    {
      s = h;
    }
    h = (((s + e)) / 2);
    if (((s + 1) == e))
    {
      break;
    }
  }
  ansN = e;
  if (((ans0 + 1) >= ansN))
  {
    ans = N;
  } else
  {
    ans = (ans0 + (((N + 1) - ansN)));
  }
  write((N - ans), "\n");
}
