// Translated from solution.cpp.

func d3(x: dynamic) -> dynamic
{
  var ret: dynamic = 0;
  while (((x % 3) == 0))
  {
    ret += 1;
    x /= 3;
  }
  return ret;
}

var v: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  v.resize(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(v[i].second);
      v[i].first = (-d3(v[i].second));
      i += 1;
    }
  }
  sort(v.begin(), v.end());
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      write(v[i].second, " ");
      i += 1;
    }
  }
}
