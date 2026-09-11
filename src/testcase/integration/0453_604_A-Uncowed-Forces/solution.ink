// Translated from solution.cpp.

func main() -> dynamic
{
  var m: dynamic = cpp_array(5);
  var w: dynamic = cpp_array(5);
  var h: dynamic = cpp_array(2);
  var score: dynamic = [500, 1000, 1500, 2000, 2500];
  {
    var i: dynamic = 0;
    while ((i < 5))
    {
      read(m[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < 5))
    {
      read(w[i]);
      i += 1;
    }
  }
  read(h[0], h[1]);
  var ans: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < 5))
    {
      var mx: dynamic = (((3 * score[i])) / 10);
      var cx: dynamic = ((((((250 - m[i])) * ((score[i] / 250))))) - (50 * w[i]));
      ans += max(mx, cx);
      i += 1;
    }
  }
  ans += ((100 * h[0]) - (50 * h[1]));
  write(ans, "\n");
  return 0;
}
