// Translated from solution.cpp.

func f(name: dynamic, arg1: dynamic)
{
  write(name, " : ", arg1, cpp_char("\n"));
}

func f(names: dynamic, arg1: dynamic, args: dynamic...)
{
  var comma = strchr((names + 1), cpp_char(","));
  (((cerr.write(names, (comma - names)) << " : ") << arg1) << " | ");
  f((comma + 1), cpp_expand(args));
}

var ans = 0;

func main()
{
  ios_base.sync_with_stdio(false);
  var n: dynamic;
  var k: dynamic;
  read(n, k);
  var data: dynamic;
  {
    var i = 0;
    while ((i < n))
    {
      var tmp: dynamic;
      read(tmp);
      data.push_back(tmp);
      i += 1;
    }
  }
  var f = data[(k - 1)];
  var i = k;
  while ((i < n))
  {
    if ((data[i] != f))
    {
      write("-1");
      return 0;
    }
    i += 1;
  }
  var tmp = 0;
  var j = (k - 2);
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
