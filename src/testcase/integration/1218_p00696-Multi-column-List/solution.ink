// Translated from solution.cpp.

var SIZE = 101;

var WORD_MAX = 1001;

var n: dynamic;

var m: dynamic;

var w: dynamic;

var p: dynamic;

var S = cpp_array(WORD_MAX);

var si: dynamic;

var T = cpp_array(SIZE, SIZE);

var pi: dynamic;

var ti: dynamic;

class Solution
{
  func init()
  {
    }
  func get_number()
  {
      var line: dynamic;
      getline(cin, line);
      var res: dynamic;
      (iss >> res);
      return res;
    }
  func input()
  {
      n = get_number();
      if ((n == 0))
      {
        return false;
      }
      m = get_number();
      w = get_number();
      p = get_number();
      var line: dynamic;
      si = 0;
      while (getline(cin, line))
      {
        if ((line == "?"))
        {
          break;
        }
        S[cpp_update(si, "++")] = line;
      }
      return true;
    }
  func add_item(s: dynamic)
  {
      var len = s.size();
      if ((len > w))
      {
        {
          var i = 0;
          while ((i < len))
          {
            var t = s.substr(i, w);
            add_item(t);
            i += w;
          }
        }
      } else
      {
        var r = (ti % n);
        var c = (ti / n);
        {
          var i = 0;
          while ((i < len))
          {
            T[pi][r][((i + (w * c)) + (p * c))] = s[i];
            i += 1;
          }
        }
        ti += 1;
        if ((ti >= (n * m)))
        {
          ti = 0;
          pi += 1;
        }
      }
    }
  func solve()
  {
      pi = 0;
      ti = 0;
      {
        var i = 0;
        while ((i < SIZE))
        {
          {
            var j = 0;
            while ((j < n))
            {
              T[i][j] = string_cpp(((m * w) + (p * ((m - 1)))), cpp_char("."));
              j += 1;
            }
          }
          i += 1;
        }
      }
      {
        var i = 0;
        while ((i < si))
        {
          var word = S[i];
          add_item(word);
          i += 1;
        }
      }
    }
  func output()
  {
      {
        var i = 0;
        while ((i <= pi))
        {
          if (((i == pi) && (ti == 0)))
          {
            i += 1;
            continue;
          }
          {
            var j = 0;
            while ((j < n))
            {
              write(T[i][j], "\n");
              j += 1;
            }
          }
          write("#", "\n");
          i += 1;
        }
      }
      write("?", "\n");
    }
  func run()
  {
      while (cpp_comma(init(), input()))
      {
        solve();
        output();
      }
      return 0;
    }
}

func main()
{
  var s: dynamic;
  return s.run();
}
