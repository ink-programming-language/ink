// Translated from solution.cpp.

var N: dynamic = cpp_uninitialized();

var xs: dynamic = cpp_uninitialized();

var ys: dynamic = cpp_uninitialized();

func generateFunc(i: dynamic, x: dynamic) -> dynamic
{
  var str: dynamic = cpp_array(1024);
  sprintf(str, "(%d*((1-abs((t-%d)))+abs((abs((t-%d))-1))))", (x / 2), i, i);
  return string_cpp(str);
}

func solve(xs: dynamic) -> dynamic
{
  var rv: dynamic = cpp_uninitialized();
  rv += generateFunc(0, xs[0]);
  {
    var i: dynamic = 1;
    while ((i < N))
    {
      rv = (((("(" + rv) + "+") + generateFunc(i, xs[i])) + ")");
      i += 1;
    }
  }
  return rv;
}

func main() -> dynamic
{
  read(N);
  {
    var i: dynamic = 0;
    while ((i < N))
    {
      var x: dynamic = cpp_uninitialized();
      var y: dynamic = cpp_uninitialized();
      var r: dynamic = cpp_uninitialized();
      read(x, y, r);
      xs.emplace_back(x);
      ys.emplace_back(y);
      i += 1;
    }
  }
  write(solve(xs), "\n");
  write(solve(ys), "\n");
  return 0;
}
