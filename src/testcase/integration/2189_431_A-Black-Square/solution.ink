// Translated from solution.cpp.

var a: dynamic;

var b: dynamic;

var c: dynamic;

var d: dynamic;

var e: dynamic;

var m: dynamic;

var n: dynamic;

var cnt: dynamic;

var ans: dynamic;

var x: dynamic;

var y: dynamic;

var arr = cpp_array(10000);

var s: dynamic;

func main()
{
  read(arr[0], arr[1], arr[2], arr[3]);
  read(s);
  {
    var i = 0;
    while ((i < s.size()))
    {
      if (((s[i] - 49) == 0))
      {
        cnt += arr[0];
      } else if (((s[i] - 49) == 1))
      {
        cnt += arr[1];
      } else if (((s[i] - 49) == 2))
      {
        cnt += arr[2];
      } else if (((s[i] - 49) == 3))
      {
        cnt += arr[3];
      }
      i += 1;
    }
  }
  write(cnt);
}
