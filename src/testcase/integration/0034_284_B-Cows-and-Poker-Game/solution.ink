// Translated from solution.cpp.

func pp(v: dynamic)
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

func pp(v: dynamic, n: dynamic)
{
  {
    var i = 0;
    while ((i < cpp_cast(n)))
    {
      write(v[i], cpp_char(" "));
      i += 1;
    }
  }
  write("\n");
}

func chmax(a: dynamic, b: dynamic)
{
  a = max(a, b);
}

func chmin(a: dynamic, b: dynamic)
{
  a = min(a, b);
}

var INF = (1 << 28);

var EPS = 1.0e-9;

var dx = [1, 0, -1, 0];

var dy = [0, -1, 0, 1];

func main(argument_0: dynamic)
{
  var N: dynamic;
  read(N);
  var line: dynamic;
  read(line);
  var A = 0;
  var F = 0;
  var I = 0;
  {
    var i = 0;
    while ((i < cpp_cast(N)))
    {
      var __cpp_switch_1 = line[i];
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
  var ans = 0;
  {
    var i = 0;
    while ((i < cpp_cast(N)))
    {
      if (((line[i] == cpp_char("A")) || (line[i] == cpp_char("I"))))
      {
        var irem = (I - (if ((line[i] == cpp_char("I"))) 1 else 0));
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
