// Translated from solution.cpp.

var mod = 1000000007;

var child0: dynamic;

var child1: dynamic;

var claw: dynamic;

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(null);
  cout.tie(null);
  var n: dynamic;
  read(n);
  var max = 0;
  {
    var i = 0;
    while ((i < n))
    {
      read(a[i]);
      if ((a[i] > max))
      {
        max = a[i];
      }
      i += 1;
    }
  }
  child0.resize((max + 1));
  child1.resize((max + 1));
  claw.resize((max + 1));
  child0[0] = 1;
  child0[1] = 1;
  child0[2] = 3;
  child1[0] = 0;
  child1[1] = 1;
  child1[2] = 1;
  claw[0] = 0;
  claw[1] = 0;
  claw[2] = 1;
  {
    var i = 3;
    while ((i < max))
    {
      claw[i] = (((child1[(i - 1)] + claw[(i - 3)])) % mod);
      child0[i] = (((child0[(i - 1)] + (2 * child1[(i - 1)]))) % mod);
      child1[i] = ((child0[(i - 1)]) % mod);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      write((((4 * claw[(a[i] - 1)])) % mod), "\n");
      i += 1;
    }
  }
  return 0;
}
