// Translated from solution.cpp.

var n: dynamic;

var m: dynamic;

var t: dynamic;

var y: dynamic;

var j: dynamic;

var l: dynamic;

var i: dynamic;

var h: dynamic;

var q: dynamic;

var q1: dynamic;

var q2: dynamic;

var p1: dynamic;

var p2: dynamic;

var k: dynamic;

var a1 = cpp_array(5);

var a2 = cpp_array(5);

var b1: dynamic;

var b2: dynamic;

func solve()
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

func init()
{
}

func answer()
{
}

func main()
{
  init();
  solve();
  answer();
  return 0;
}
