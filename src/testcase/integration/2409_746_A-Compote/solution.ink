// Translated from solution.cpp.

func main()
{
  var i = 0;
  var k = 0;
  var m: dynamic;
  var s1 = 0;
  var j: dynamic;
  var n: dynamic;
  var r = 0;
  var l = 0;
  var l1 = 0;
  var c = 0;
  var t: dynamic;
  var d = 0;
  var na: dynamic;
  var nb: dynamic;
  read(n, m, k);
  {
    i = 1;
    while ((i <= n))
    {
      if (((m >= (2 * i)) && (k >= (4 * i))))
      {
        d = i;
      }
      i += 1;
    }
  }
  write(((d) * 7));
  return 0;
}
