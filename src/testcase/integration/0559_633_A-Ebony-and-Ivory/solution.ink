// Translated from solution.cpp.

var a: dynamic;

var b: dynamic;

var c: dynamic;

func main()
{
  var a: dynamic;
  var b: dynamic;
  var c: dynamic;
  var i: dynamic;
  var j: dynamic;
  var k: dynamic;
  read(a, b, c);
  {
    i = 0;
    while ((i <= (c / a)))
    {
      if (((((c - (i * a))) % b) == 0))
      {
        write("Yes");
        return 0;
      }
      i += 1;
    }
  }
  write("No");
}
