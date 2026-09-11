// Translated from solution.cpp.

var PB: dynamic = cpp_expression("/********");

var UB: dynamic = cpp_expression("/**********");

var LB: dynamic = cpp_expression("/**********");

var BS: dynamic = cpp_expression("/************");

var MP: dynamic = cpp_expression("/********");

var EB: dynamic = cpp_expression("/***********");

var endl: dynamic = cpp_expression("/***");

var PI: dynamic = cpp_expression("/********************************");

var MOD: dynamic = cpp_expression("/*********");

var F: dynamic = cpp_expression("/****");

var S: dynamic = cpp_expression("/*****");

var umap: dynamic = cpp_expression("/************");

var uset: dynamic = cpp_expression("/************");

func dec(x: dynamic) -> dynamic
{
  return cpp_expression("/*********************");
}

func lcm(a: dynamic, b: dynamic) -> dynamic
{
  return cpp_expression("/***************");
}

func solution() -> dynamic
{
  var ans: dynamic = true;
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, k);
  var a: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  read(a, b);
  var fa: dynamic = cpp_construct(26, 0);
  var fb: dynamic = cpp_construct(26, 0);
  for (var it: dynamic in a)
  {
    fa[(it - cpp_char("a"))] += 1;
  }
  for (var it: dynamic in b)
  {
    fb[(it - cpp_char("a"))] += 1;
  }
  {
    var i: dynamic = 0;
    while ((i < 26))
    {
      if ((fa[i] < fb[i]))
      {
        ans = false;
        break;
      } else
      {
        var diff: dynamic = abs((fa[i] - fb[i]));
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

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  var t: dynamic = cpp_uninitialized();
  read(t);
  while (cpp_update(t, "--"))
  {
    solution();
  }
  return 0;
}
