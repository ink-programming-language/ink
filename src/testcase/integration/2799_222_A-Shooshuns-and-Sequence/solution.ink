// Translated from solution.cpp.

func f(name: dynamic, arg1: dynamic) -> dynamic
{
  write(name, " : ", arg1, cpp_char("\n"));
}

func f(names: dynamic, arg1: dynamic, args: dynamic...) -> dynamic
{
  var comma: dynamic = strchr((names + 1), cpp_char(","));
  (((cerr.write(names, (comma - names)) << " : ") << arg1) << " | ");
  f((comma + 1), cpp_expand(args));
}

var ans: dynamic = 0;

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, k);
  var data: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var tmp: dynamic = cpp_uninitialized();
      read(tmp);
      data.push_back(tmp);
      i += 1;
    }
  }
  var f: dynamic = data[(k - 1)];
  var i: dynamic = k;
  while ((i < n))
  {
    if ((data[i] != f))
    {
      write("-1");
      return 0;
    }
    i += 1;
  }
  var tmp: dynamic = 0;
  var j: dynamic = (k - 2);
  while ((j >= 0))
  {
    if ((data[j] != f))
    {
      tmp = (j + 1);
      break;
    }
    j -= 1;
  }
  write(tmp);
  return 0;
}
