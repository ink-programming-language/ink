// Translated from solution.cpp.

var N: dynamic;

var xs: dynamic;

var ys: dynamic;

func generateFunc(i: dynamic, x: dynamic)
{
  var str = cpp_array(1024);
  sprintf(str, "(%d*((1-abs((t-%d)))+abs((abs((t-%d))-1))))", (x / 2), i, i);
  return string_cpp(str);
}

func solve(xs: dynamic)
{
  var rv: dynamic;
  rv += generateFunc(0, xs[0]);
  {
    var i = 1;
    while ((i < N))
    {
      rv = (((("(" + rv) + "+") + generateFunc(i, xs[i])) + ")");
      i += 1;
    }
  }
  return rv;
}

func main()
{
  read(N);
  {
    var i = 0;
    while ((i < N))
    {
      var x: dynamic;
      var y: dynamic;
      var r: dynamic;
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
