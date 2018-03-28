# LycheeCloudRemo
GR-LYCHEEを使ったクラウドリモコンのデモです。
httpsのGETに対応しているクラウドサーバーからコマンド文字列を受け取り、コマンド文字列に応じた処理を行います。

クラウドサーバー側は、httpsレスポンスとして１行のコマンド文字列（例: "ON March 25, 2018 at 05:38PM"）を返すことを想定しています。
GR-LYCHEEは、サーバーをポーリングしながら監視し、そのコマンド文字列に応じた処理を行います。


## 事前準備
Google HomeやAmazon Echoなどの音声アシスタントとIFTTTとGoogle Firebaseを組み合わせることで、音声アシスタントからGR-LYCHEEを操作することができます。

### Google Firebaseの設定
1. [Google Firebase コンソール](https://console.firebase.google.com/)にログインします
2. プロジェクトを追加します。
    - プロジェクト名:任意の名称を入力 (例:xxxx-cloudremo)
    - 国/地域:"日本"を選択
3. 認証トークンを取得します。
    - Settingsアイコン（ギアアイコン）を押し、プロジェクトの設定を選択します。
    - 設定画面から、サービスアカウントタブを選択します。
    - データベースのシークレットを選択します。
    - 伏せ字となっているため、"表示する"リンクを押して表示します。
    - 表示されたシークレットをメモします。
    - シークレットは秘密情報なため、GitHubなどに公開しないように注意します。
3. Develop - Database を選択します。
    - Databaseの選択画面で、RealTime Database を選択します。
    - 画面内に表示されているURLをメモします。(例:https://xxxx-cloudremo.firebaseio.com/)

### IFTTTの設定
ここではGoogle Assistantの場合を例としますが、Amazon Alexaも同様です。

1. New Appletを選択します。
2. "this"にて、"Google Assistant"を選択します。
    - "Say a simple phrase"を選択します。
    - "What do you want to say?"に音声コマンドを登録します。
        - 例） ライトをつけて
    - "What's another way to say it?"と"And another way?"にも別の言い回しでコマンドを登録しておくことができます。
        - 例) シトラスのライトをつけて
        - 例） 電気をつけて
    - "What do you want the Assistant to say in response?"に適当な応答文を設定します
        - 例) つけます
    - "Language"は"Japanese"を選択します。
3. "that"にて、"Webhooks"を選択します。
    - "URL"にFirebaseの設定にて取得したURLを設定します。その際、word.jsonとauth=シークレットをつなげてください。
        - 例） https://xxxx-cloudremo.firebaseio.com/word.json?auth=aaaaaaa
    - "Method"は"PUT"を選択してください。
    - "Content Type"は"application/json"を選択してください。
    - "Body"は任意のコマンド文字列を入力してください。文字列はダブルコーテーションで囲んでください。
        - 例） "ON {{CreatedAt}}"
        - 例） "OFF {{CreatedAt}}"
4. 同様に、他の音声コマンド（例:ライトを消して）も別のAppletとして登録してください。

### 動作確認
1. 音声アシスト端末でコマンドを実行します。
    - 例) OK Google、ライトをつけて
    - 例） Alexa、ライトをつけて、をトリガー
2. ブラウザで、FirebaseのURL(IFTTTで設定したURLと同じURL)にアクセスします。
    - 例） https://xxxx-cloudremo.firebaseio.com/word.json?auth=aaaaaaa
3. 音声認識した結果のコマンド文字列がブラウザに表示されたことを確認します。


## 使い方
### 必要なもの
- GR-LYCHEE
- LED　:　動作確認用。１3ピンとGNDに接続してください。

### ビルド
mbed_app.jsonを開き、wifiのアクセスポイントやクラウドの接続先情報を各自の環境にあわせて修正してください。

- wifi-ssid : wifiのSSIDを設定
- wifi-password : wifiのパスワードを設定
- server-url : 接続先のクラウドのURL
    - 例） https://xxxx-cloudremo.firebaseio.com/word.json?auth=aaaaaaa

### 実行
GR-LYCHEEをPCに接続し、ビルドしたファイルをMBEDドライブに書き込んでください。

実行中はGR-LYCHEEのLED1がゆっくりと点滅します。異常終了するとLED1が高速に点滅します。何らかの理由で通信に失敗した場合、30秒待ってから再度処理を実行します。
