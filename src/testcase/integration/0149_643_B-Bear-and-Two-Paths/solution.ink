// Translated from solution.cpp.

func operator_shift_left(out: dynamic, v: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < v.size()))
    {
      ((out << v[i]) << " ");
      i += 1;
    }
  }
  (out << "\n");
  return out;
}

func operator_shift_left(out: dynamic, s: dynamic) -> dynamic
{
  for (var e: dynamic in s)
  {
    ((out << e) << " ");
  }
  (out << "\n");
  return out;
}

func operator_shift_left(out: dynamic, p: dynamic) -> dynamic
{
  (((((out << "(") << p.first) << ", ") << p.second) << ") ");
  return out;
}

func operator_shift_left(out: dynamic, v: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < v.size()))
    {
      write(v[i]);
      i += 1;
    }
  }
  (out << "\n");
  return out;
}

func operator_shift_left(out: dynamic, v: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < v.size()))
    {
      {
        var j: dynamic = 0;
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

func operator_shift_left(out: dynamic, v: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < v.size()))
    {
      (out << v[i]);
      i += 1;
    }
  }
  (out << "\n");
  return out;
}

func solve() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, k);
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var c: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  read(a, b, c, d);
  if (((n == 4) || (k < (n + 1))))
  {
    write("-1\n");
    return;
  }
  write(a, " ", c, " ");
  var middle_vertices: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 1;
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
    var i: dynamic = 1;
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

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var t: dynamic = cpp_uninitialized();
  t = 1;
  while (cpp_update(t, "--"))
  {
    solve();
  }
  return 0;
}
