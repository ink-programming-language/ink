// Translated from solution.cpp.

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var f: dynamic = 1;
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_array(1010);
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(r[i].first);
      r[i].second = (i + 1);
      i += 1;
    }
  }
  sort(r, (r + n));
  {
    var i: dynamic = 1;
    while ((i < (n - 2)))
    {
      if ((r[i].first != r[(i + 1)].first))
      {
        f = 0;
        break;
      }
      i += 1;
    }
  }
  if (((((n == 1) | f) && (r[0].first == r[(n - 1)].first)) && (r[(n - 2)].first == r[(n - 1)].first)))
  {
    write("Exemplary pages.\n");
  } else if ((f && ((((n == 2) && ((((r[0].first + r[(n - 1)].first)) % 2) == 0)) || ((r[(n - 1)].first + r[0].first) == (2 * r[(n - 2)].first))))))
  {
    m = (r[(n - 1)].first - r[0].first);
    a = r[0].second;
    b = r[(n - 1)].second;
    write((m / 2), " ml. from cup #", a, " to cup #", b, ".\n");
  } else
  {
    write("Unrecoverable configuration.\n");
  }
}
