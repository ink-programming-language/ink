// Translated from solution.cpp.

var n: dynamic;

var a = cpp_array(50);

var b = cpp_array(50);

func pour(p: dynamic, q: dynamic, rest: dynamic)
{
  if ((!rest))
  {
    return pourAll((p + 1));
  }
  if ((q == n))
  {
    return false;
  }
  if (((!b[q]) && (a[q] <= rest)))
  {
    b[q] = true;
    if (pour(p, (q + 1), (rest - a[q])))
    {
      return true;
    }
    b[q] = false;
  }
  return pour(p, (q + 1), rest);
}

func pourAll(p: dynamic)
{
  if ((p == n))
  {
    return true;
  }
  if ((!b[p]))
  {
    return false;
  }
  if (pour(p, (p + 1), a[p]))
  {
    return true;
  }
  return pourAll((p + 1));
}

func solve()
{
  sort(a, (a + n), greater());
  b[0] = true;
  {
    var i = 1;
    while ((i < n))
    {
      b[i] = false;
      i += 1;
    }
  }
  return pourAll(0);
}

func main()
{
  var ans: dynamic;
  while (1)
  {
    read(n);
    if ((!n))
    {
      break;
    }
    {
      var i = 0;
      while ((i < n))
      {
        read(a[i]);
        i += 1;
      }
    }
    ans = solve();
    write((if (ans) "YES" else "NO"), "\n");
  }
  return 0;
}
