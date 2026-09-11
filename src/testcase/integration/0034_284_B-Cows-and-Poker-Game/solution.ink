// Translated from solution.cpp.

func pp(v: dynamic) -> dynamic
{
  {
    typeof((v).begin()) = (v).begin();
    while ((it != (v).end()))
    {
      write((*it), cpp_char(" "));
      it += 1;
    }
  }
  write("\n");
}

func pp(v: dynamic, n: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < cpp_cast(n)))
    {
      write(v[i], cpp_char(" "));
      i += 1;
    }
  }
  write("\n");
}

func chmax(a: dynamic, b: dynamic) -> dynamic
{
  a = max(a, b);
}

func chmin(a: dynamic, b: dynamic) -> dynamic
{
  a = min(a, b);
}

var INF: dynamic = (1 << 28);

var EPS: dynamic = 1.0e-9;

var dx: dynamic = [1, 0, -1, 0];

var dy: dynamic = [0, -1, 0, 1];

func main(argument_0: dynamic) -> dynamic
{
  var N: dynamic = cpp_uninitialized();
  read(N);
  var line: dynamic = cpp_uninitialized();
  read(line);
  var A: dynamic = 0;
  var F: dynamic = 0;
  var I: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < cpp_cast(N)))
    {
      var __cpp_switch_1: dynamic = line[i];
      if (__cpp_switch_1 == cpp_char("A"))
      {
        A += 1;
        break;
      }
      else if (__cpp_switch_1 == cpp_char("F"))
      {
        F += 1;
        break;
      }
      else if (__cpp_switch_1 == cpp_char("I"))
      {
        I += 1;
        break;
      }
      i += 1;
    }
  }
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < cpp_cast(N)))
    {
      if (((line[i] == cpp_char("A")) || (line[i] == cpp_char("I"))))
      {
        var irem: dynamic = (I - ( ((line[i] == cpp_char("I"))) ? 1 : 0));
        if ((irem <= 0))
        {
          ans += 1;
        }
      }
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
