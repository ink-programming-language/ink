// Translated from solution.cpp.

func f(name: dynamic, arg1: dynamic) -> dynamic
{
  write(name, ": ", arg1, "\n");
}

func f(names: dynamic, arg1: dynamic, args: dynamic...) -> dynamic
{
  var comma: dynamic = strchr((names + 1), cpp_char(","));
  (((cerr.write(names, (comma - names)) << ": ") << arg1) << " |");
  f((comma + 1), cpp_expand(args));
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  read(n);
  var a: dynamic = cpp_array(n);
  var in_cpp: dynamic = 0;
  var s1: dynamic = 0;
  var s2: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(a[i]);
      s1 += a[i];
      i += 1;
    }
  }
  read(m);
  var b: dynamic = cpp_array(m);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      read(b[i]);
      s2 += b[i];
      i += 1;
    }
  }
  var c: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  c.resize((((n * ((n - 1)))) / 2));
  d.resize((((m * ((m - 1)))) / 2));
  var v: dynamic = LONG_MAX;
  var x1: dynamic = -1;
  var x2: dynamic = -1;
  var y1: dynamic = -1;
  var y2: dynamic = -1;
  v = abs((s1 - s2));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = (i + 1);
        while ((j < n))
        {
          c[in_cpp] = [((2 * a[i]) + (2 * a[j])), in_cpp];
          in_cpp += 1;
          j += 1;
        }
      }
      i += 1;
    }
  }
  in_cpp = 0;
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      {
        var j: dynamic = (i + 1);
        while ((j < m))
        {
          d[in_cpp] = [((2 * b[i]) + (2 * b[j])), in_cpp];
          in_cpp += 1;
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = 0;
        while ((j < m))
        {
          if ((abs(((s1 - s2) + (2 * ((b[j] - a[i]))))) <= v))
          {
            v = abs(((s1 - s2) + (2 * ((b[j] - a[i])))));
            x1 = i;
            y1 = j;
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  var it: dynamic = cpp_uninitialized();
  sort(d.begin(), d.end());
  {
    var i: dynamic = 0;
    while ((i < (((n * ((n - 1)))) / 2)))
    {
      it = lower_bound(d.begin(), d.end(), make_pair(((s2 - s1) + c[i].first), cpp_cast(-1)));
      if ((it == d.end()))
      {
        in_cpp = ((((m * ((m - 1)))) / 2) - 1);
      } else
      {
        in_cpp = (it - d.begin());
      }
      if ((((in_cpp >= 0) && (d.size() > 0)) && (abs((((s2 - s1) + c[i].first) - d[in_cpp].first)) < v)))
      {
        v = abs((((s2 - s1) + c[i].first) - d[in_cpp].first));
        x2 = c[i].second;
        y2 = d[in_cpp].second;
      }
      if (((((in_cpp - 1) >= 0) && (d.size() > 0)) && (abs((((s2 - s1) + c[i].first) - d[(in_cpp - 1)].first)) < v)))
      {
        v = abs((((s2 - s1) + c[i].first) - d[(in_cpp - 1)].first));
        x2 = c[i].second;
        y2 = d[(in_cpp - 1)].second;
      }
      i += 1;
    }
  }
  write(v, "\n");
  if ((y1 == -1))
  {
    write(0, "\n");
    return 0;
  }
  if ((y2 == -1))
  {
    write(1, "\n");
    write((x1 + 1), " ", (y1 + 1), "\n");
    return 0;
  }
  in_cpp = 0;
  var v1: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var j: dynamic = (i + 1);
        while ((j < n))
        {
          if ((in_cpp == x2))
          {
            x1 = i;
            x2 = j;
            v1 = 1;
            break;
          }
          in_cpp += 1;
          j += 1;
        }
      }
      if (v1)
      {
        break;
      }
      i += 1;
    }
  }
  in_cpp = 0;
  v1 = 0;
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      {
        var j: dynamic = (i + 1);
        while ((j < m))
        {
          if ((in_cpp == y2))
          {
            y1 = i;
            y2 = j;
            v1 = 1;
            break;
          }
          in_cpp += 1;
          j += 1;
        }
      }
      if (v1)
      {
        break;
      }
      i += 1;
    }
  }
  write(2, "\n");
  write((x1 + 1), " ", (y1 + 1), "\n");
  write((x2 + 1), " ", (y2 + 1), "\n");
  return 0;
}
