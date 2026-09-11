// Translated from solution.cpp.

var a: dynamic = cpp_uninitialized();

var b: dynamic = cpp_uninitialized();

var c: dynamic = cpp_uninitialized();

var d: dynamic = cpp_uninitialized();

var e: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var cnt: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

var x: dynamic = cpp_uninitialized();

var y: dynamic = cpp_uninitialized();

var arr: dynamic = cpp_array(10000);

var s: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(arr[0], arr[1], arr[2], arr[3]);
  read(s);
  {
    var i: dynamic = 0;
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
