// Translated from solution.cpp.

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var t: dynamic = cpp_uninitialized();

var y: dynamic = cpp_uninitialized();

var j: dynamic = cpp_uninitialized();

var l: dynamic = cpp_uninitialized();

var i: dynamic = cpp_uninitialized();

var h: dynamic = cpp_uninitialized();

var q: dynamic = cpp_uninitialized();

var q1: dynamic = cpp_uninitialized();

var q2: dynamic = cpp_uninitialized();

var p1: dynamic = cpp_uninitialized();

var p2: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var a1: dynamic = cpp_array(5);

var a2: dynamic = cpp_array(5);

var b1: dynamic = cpp_uninitialized();

var b2: dynamic = cpp_uninitialized();

func solve() -> dynamic
{
  {
    j = 1;
    while ((j <= 4))
    {
      scanf("%d%d", (&a1[j]), (&a2[j]));
      j += 1;
    }
  }
  if (((((((a2[1] > a1[3]) && (a1[2] > a2[4])) && (a2[1] > a1[4])) && (a1[2] > a2[3]))) || (((((a2[2] > a1[3]) && (a1[1] > a2[4])) && (a2[2] > a1[4])) && (a1[1] > a2[3])))))
  {
    b1 = 1;
  }
  if (((((((a1[2] < a2[4]) && (a2[1] < a1[3]))) || (((a1[2] < a2[3]) && (a2[1] < a1[4]))))) && (((((a1[1] < a2[4]) && (a2[2] < a1[3]))) || (((a1[1] < a2[3]) && (a2[2] < a1[4])))))))
  {
    b2 = 1;
  }
  if (b1)
  {
    printf("Team 1\n");
  }
  if (b2)
  {
    printf("Team 2\n");
  }
  if (((!b1) && (!b2)))
  {
    printf("Draw\n");
  }
}

func init() -> dynamic
{
}

func answer() -> dynamic
{
}

func main() -> dynamic
{
  init();
  solve();
  answer();
  return 0;
}
