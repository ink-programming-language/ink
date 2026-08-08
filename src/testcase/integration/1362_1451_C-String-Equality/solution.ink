// Translated from solution.cpp.

var PB = cpp_expression("/********");

var UB = cpp_expression("/**********");

var LB = cpp_expression("/**********");

var BS = cpp_expression("/************");

var MP = cpp_expression("/********");

var EB = cpp_expression("/***********");

var endl = cpp_expression("/***");

var PI = cpp_expression("/********************************");

var MOD = cpp_expression("/*********");

var F = cpp_expression("/****");

var S = cpp_expression("/*****");

var umap = cpp_expression("/************");

var uset = cpp_expression("/************");

func dec(x: dynamic)
{
  return cpp_expression("/*********************");
}

func lcm(a: dynamic, b: dynamic)
{
  return cpp_expression("/***************");
}

func solution()
{
  var ans = true;
  var n: dynamic;
  var k: dynamic;
  read(n, k);
  var a: dynamic;
  var b: dynamic;
  read(a, b);
  var fa = cpp_construct(26, 0);
  var fb = cpp_construct(26, 0);
  for (var it in a)
  {
    fa[(it - cpp_char("a"))] += 1;
  }
  for (var it in b)
  {
    fb[(it - cpp_char("a"))] += 1;
  }
  {
    var i = 0;
    while ((i < 26))
    {
      if ((fa[i] < fb[i]))
      {
        ans = false;
        break;
      } else
      {
        var diff = abs((fa[i] - fb[i]));
        if (((diff % k) != 0))
        {
          ans = false;
          break;
        } else
        {
          fa[i] -= diff;
          fa[(i + 1)] += diff;
        }
      }
      i += 1;
    }
  }
  if (ans)
  {
    write("Yes\n");
  } else
  {
    write("No\n");
  }
}

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var t: dynamic;
  read(t);
  while (cpp_update(t, "--"))
  {
    solution();
  }
  return 0;
}
