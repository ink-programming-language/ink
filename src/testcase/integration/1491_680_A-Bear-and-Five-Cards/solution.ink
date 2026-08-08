// Translated from solution.cpp.

var i: dynamic;

var j: dynamic;

var k: dynamic;

var a: dynamic;

var b: dynamic;

var n: dynamic;

func main()
{
  var n: dynamic;
  var cc = [0];
  var m = 0;
  var s = 0;
  {
    i = 0;
    while ((i < 5))
    {
      read(n);
      s += n;
      cc[n] += 1;
      if (((cc[n] == 3) || (cc[n] == 2)))
      {
        m = max(m, (cc[n] * n));
      }
      i += 1;
    }
  }
  write((s - m), "\n");
}
