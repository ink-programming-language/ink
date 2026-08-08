// Translated from solution.cpp.

func operator_shift_left(out: dynamic, v: dynamic)
{
  {
    var i = 0;
    while ((i < v.size()))
    {
      ((out << v[i]) << " ");
      i += 1;
    }
  }
  (out << "\n");
  return out;
}

func operator_shift_left(out: dynamic, s: dynamic)
{
  for (var e in s)
  {
    ((out << e) << " ");
  }
  (out << "\n");
  return out;
}

func operator_shift_left(out: dynamic, p: dynamic)
{
  (((((out << "(") << p.first) << ", ") << p.second) << ") ");
  return out;
}

func operator_shift_left(out: dynamic, v: dynamic)
{
  {
    var i = 0;
    while ((i < v.size()))
    {
      write(v[i]);
      i += 1;
    }
  }
  (out << "\n");
  return out;
}

func operator_shift_left(out: dynamic, v: dynamic)
{
  {
    var i = 0;
    while ((i < v.size()))
    {
      {
        var j = 0;
        while ((j < v[i].size()))
        {
          ((out << v[i][j]) << " ");
          j += 1;
        }
      }
      (out << "\n");
      i += 1;
    }
  }
  return out;
}

func operator_shift_left(out: dynamic, v: dynamic)
{
  {
    var i = 0;
    while ((i < v.size()))
    {
      (out << v[i]);
      i += 1;
    }
  }
  (out << "\n");
  return out;
}

func solve()
{
  var n: dynamic;
  var k: dynamic;
  read(n, k);
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  var d: dynamic;
  read(a, b, c, d);
  if (((n == 4) || (k < (n + 1))))
  {
    write("-1\n");
    return;
  }
  write(a, " ", c, " ");
  var middle_vertices: dynamic;
  {
    var i = 1;
    while ((i <= n))
    {
      if (((((i != a) && (i != b)) && (i != c)) && (i != d)))
      {
        write(i, " ");
      }
      i += 1;
    }
  }
  write(d, " ", b, "\n");
  write(c, " ", a, " ");
  {
    var i = 1;
    while ((i <= n))
    {
      if (((((i != a) && (i != b)) && (i != c)) && (i != d)))
      {
        write(i, " ");
      }
      i += 1;
    }
  }
  write(b, " ", d, "\n");
  return;
}

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var t: dynamic;
  t = 1;
  while (cpp_update(t, "--"))
  {
    solve();
  }
  return 0;
}
