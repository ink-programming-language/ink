// Translated from solution.cpp.

var INF: dynamic = (1 << 30);

var MAX: dynamic = 10000;

var mod: dynamic = 1000000007;

var pi: dynamic = 3.141592653589;

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  read(n);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(v[i]);
      i += 1;
    }
  }
  var q: dynamic = cpp_uninitialized();
  read(q);
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < q))
    {
      read(l, r, t);
      swap_ranges((v.begin() + l), (v.begin() + r), (v.begin() + t));
      i += 1;
    }
  }
  printVec(v);
  return 0;
}

func printVec(vec: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < vec.size()))
    {
      if (i)
      {
        write(cpp_char(" "));
      }
      write(vec[i]);
      i += 1;
    }
  }
  write(cpp_char("\n"));
}
